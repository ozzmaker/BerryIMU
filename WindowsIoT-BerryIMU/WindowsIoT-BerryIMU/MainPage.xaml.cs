using System;
using System.Threading;
using Windows.UI.Xaml.Controls;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
 
 
namespace BerryImu
{
    struct Gyroscope
    {
        public double rawX;
        public double rawY;
        public double rawZ;
    };
    struct Accelerometer
    {
        public double rawX;
        public double rawY;
        public double rawZ;
    };
    struct Magnetometer
    {
        public double rawX;
        public double rawY;
        public double rawZ;
        public double headingInDegrees;
        public double headingInDegreesTiltCompensated;
    };
 
    public sealed partial class MainPage : Page
    {
        private I2cDevice i2cDeviceGyroscope, i2cDeviceAccelerometerMagnetometer;
        private Timer periodicTimer;
 
        const double gyroscopeAngularRateSensitivity = 0.070; // deg/s/LSB (Angular rate FS = ±2000 degrees per second)
        const double complementaryFilterConstant = 0.03;     // Complementary filter constant
        const int loopDeltaInMilliseconds = 100;         //DT is the loop delta in milliseconds.
 
        double gyroscopeAngleX = 0.0;
        double gyroscopeAngleY = 0.0;
        double gyroscopeAngleZ = 0.0;
 
        bool isBerryImuUpsideDown = false;              //set to false if BerryIMU is up the correct way. This is when the skull logo is facing down.
 
        double pitch, roll;
 
        public MainPage()
        {
            this.InitializeComponent();
 
            // Register for the unloaded event so we can clean up upon exit
            Unloaded += MainPage_Unloaded;
 
            // Initialize the I2C bus, Gyroscope, and Timer
            InitI2CBerryIMU();
        }
 
        private async void InitI2CBerryIMU()
        {
            string allIc2Controllers = I2cDevice.GetDeviceSelector();             // Get a selector string that will return all I2C controllers on the system
            var discoveredI2cDevices = await DeviceInformation.FindAllAsync(allIc2Controllers);    // Find the I2C bus controller device with our selector string
            if (discoveredI2cDevices.Count == 0)
            {
                DisplayTextStatus.Text = "No I2C controllers were found on the system";
                return;
            }
 
            // Specify I2C slave addresses for the Gyro and Accelerometer on BerryIMU. 
            //      Note; the magenetormeter(compass) uses the same address as the accelerometer.
            //      We will use the ACC_ADDRESS I2C settings to access both the accelerometer and the magnetometer.
            var i2cConnectionSettingsGyroscope = new I2cConnectionSettings(LSM9DS0.GYR_ADDRESS);
            var i2cConnectionSettingsAccelerometerMagnetometer = new I2cConnectionSettings(LSM9DS0.ACC_ADDRESS);
 
            // Enable 400kHz I2C bus speed
            i2cConnectionSettingsGyroscope.BusSpeed = I2cBusSpeed.FastMode; // 400kHz
            i2cConnectionSettingsAccelerometerMagnetometer.BusSpeed = I2cBusSpeed.FastMode; // 400kHz
 
            // Create I2cDevices with our selected bus controller and I2C settings
            i2cDeviceGyroscope = await I2cDevice.FromIdAsync(discoveredI2cDevices[0].Id, i2cConnectionSettingsGyroscope);
            i2cDeviceAccelerometerMagnetometer = await I2cDevice.FromIdAsync(discoveredI2cDevices[0].Id, i2cConnectionSettingsAccelerometerMagnetometer);
 
            // Enable the gyrscope
            WriteByteToGyroscope(LSM9DS0.CTRL_REG1_G, 0x0F);    // Normal power mode, all axes enabled)
            WriteByteToGyroscope(LSM9DS0.CTRL_REG4_G, 0x30);    // Continuos update, 2000 degrees/s full scale
 
            //Enable the accelerometer
            WriteByteToAccelerometer(LSM9DS0.CTRL_REG1_XM, 0x67);    // z,y,x axis enabled, continuous update,  100Hz data rate
            WriteByteToAccelerometer(LSM9DS0.CTRL_REG2_XM, 0x20);    // +/- 16G full scale
 
            // Enable the magnetometer
            WriteByteToMagnetometer(LSM9DS0.CTRL_REG5_XM, 0xF0); // Temp enable, Magnetometer data rate = 50Hz
            WriteByteToMagnetometer(LSM9DS0.CTRL_REG6_XM, 0x60); // +/-12 gauss full scale
            WriteByteToMagnetometer(LSM9DS0.CTRL_REG7_XM, 0x0);  // Continuous - conversion mode
 
            // Now that everything is initialized, create a timer so we read data every DT
            periodicTimer = new Timer(this.TimerCallback, null, 0, loopDeltaInMilliseconds);
        }
 
        private void MainPage_Unloaded(object sender, object args)
        {
            // Cleanup
            i2cDeviceGyroscope.Dispose();
            i2cDeviceAccelerometerMagnetometer.Dispose();
        }
 
        private void TimerCallback(object state)
        {
            // These strings are used to store the readings from the sensors.
            string textGyroscopeRawX, textGyroscopeRawY, textGyroscopeRawZ;
            string textAccelerometerRawX, textAccelerometerRawY, textAccelerometerRawZ;
            string textMagnetometerRawX, textMagnetometerRawY, textMagnetometerRawZ;
            string textGyroscopeAngleX, textGyroscopeAngleY, textGyroscopeAngleZ;
            string textAccelerometerangleX, textAccelerometerAngleY;
            String textHeading, textHeadingTiltCompensated;
            string textFusedX, textFusedY;
            string textStatus;
 
            double complimentaryFilterFusedAngleX = 0.0;      // Fused X angle
            double complimentaryFilterFusedAngleY = 0.0;      // Fused Y angle
            double gyroscopeDegreesPerSecondX = 0.0;    // [deg/s]
            double gyroscopeDegreesPerSecondY = 0.0;    // [deg/s]
            double gyroscopeDegreesPerSecondZ = 0.0;    // [deg/s]
 
            try
            {
                Gyroscope gyroscopeReadings = GetGyroscopeReadings();
 
                //Convert Gyro raw data to degrees/s
                gyroscopeDegreesPerSecondX = gyroscopeReadings.rawX * gyroscopeAngularRateSensitivity;
                gyroscopeDegreesPerSecondY = gyroscopeReadings.rawY * gyroscopeAngularRateSensitivity;
                gyroscopeDegreesPerSecondZ = gyroscopeReadings.rawZ * gyroscopeAngularRateSensitivity;
 
                //Calculate the angles from the Gyroscope
                gyroscopeAngleX += gyroscopeDegreesPerSecondX * loopDeltaInMilliseconds / 1000;
                gyroscopeAngleY += gyroscopeDegreesPerSecondY * loopDeltaInMilliseconds / 1000;
                gyroscopeAngleZ += gyroscopeDegreesPerSecondZ * loopDeltaInMilliseconds / 1000;
 
                Accelerometer accelerometerReadings = GetAccelerometerReadings();
 
                //Convert Accelerometer values to degrees
                var accelerometerAngleInDegreesX = (Math.Atan2(accelerometerReadings.rawY, accelerometerReadings.rawZ) + Math.PI) * (180.0 / Math.PI);
                var accelerometerAngleInDegreesY = (Math.Atan2(accelerometerReadings.rawZ, accelerometerReadings.rawX) + Math.PI) * (180.0 / Math.PI);
 
                Magnetometer magnetometerReadings = GetMagnetometerReadings();
 
                magnetometerReadings.headingInDegrees = (Math.Atan2(magnetometerReadings.rawY, magnetometerReadings.rawX)) * (180.0 / Math.PI);
 
                // Have our heading between 0 and 360
                if (magnetometerReadings.headingInDegrees < 0)
                    magnetometerReadings.headingInDegrees += 360;
 
                if (isBerryImuUpsideDown)
                {
                    if (accelerometerAngleInDegreesX > 180)
                    {
                        accelerometerAngleInDegreesX -= 360.0;
                        accelerometerAngleInDegreesY -= 90;
                    }
 
                    if (accelerometerAngleInDegreesY > 180)
                    {
                        accelerometerAngleInDegreesY -= 360.0;
                    }
 
                    //Only needed if the heading value does not increase when the magnetometer is rotated clockwise
                    magnetometerReadings.rawY = -magnetometerReadings.rawY;
                }
                else
                {
                    accelerometerAngleInDegreesX -= 180.0;
 
                    if (accelerometerAngleInDegreesY > 90)
                    {
                        accelerometerAngleInDegreesY -= 270;
                    }
 
                    else
                    {
                        accelerometerAngleInDegreesY += 90;
                    }
                }
 
                //Complementary filter used to combine the accelerometer and gyro values.
                complimentaryFilterFusedAngleX = complementaryFilterConstant * (complimentaryFilterFusedAngleX + (gyroscopeDegreesPerSecondX * loopDeltaInMilliseconds / 1000)) + (1.0 - complementaryFilterConstant) * accelerometerAngleInDegreesX;
                complimentaryFilterFusedAngleY = complementaryFilterConstant * (complimentaryFilterFusedAngleY + (gyroscopeDegreesPerSecondY * loopDeltaInMilliseconds / 1000)) + (1.0 - complementaryFilterConstant) * accelerometerAngleInDegreesY;
 
                // Tilt compensated heading calculations
                //
                // Normalize accelerometer raw values.
                var accelerometerNormalisedReadingX = accelerometerReadings.rawX / Math.Sqrt(Math.Pow(accelerometerReadings.rawX, 2) + Math.Pow(accelerometerReadings.rawY, 2) + Math.Pow(accelerometerReadings.rawZ, 2));
                var accelerometerNormalisedReadingY = accelerometerReadings.rawY / Math.Sqrt(Math.Pow(accelerometerReadings.rawX, 2) + Math.Pow(accelerometerReadings.rawY, 2) + Math.Pow(accelerometerReadings.rawZ, 2));
 
                // Calculate pitch and roll
                if (isBerryImuUpsideDown)
                {
                    // Use these four lines when the IMU is upside down. Skull logo is facing up
                    accelerometerNormalisedReadingX = -accelerometerNormalisedReadingX;     //flip Xnorm as the IMU is upside down
                    accelerometerNormalisedReadingY = -accelerometerNormalisedReadingY;     //flip Ynorm as the IMU is upside down
 
                    pitch = Math.Asin(accelerometerNormalisedReadingX);
                    roll = Math.Asin(accelerometerNormalisedReadingY / Math.Cos(pitch));
                }
                else
                {
                    // Use these two lines when the IMU is up the right way. Skull logo is facing down
                    pitch = Math.Asin(accelerometerNormalisedReadingX);
                    roll = -Math.Asin(accelerometerNormalisedReadingY / Math.Cos(pitch));
                }
 
                // Calculate the new tilt compensated values
                double magnetometerTiltCompensatedX = magnetometerReadings.rawX * Math.Cos(pitch) + magnetometerReadings.rawZ * Math.Sin(pitch);
                double magnetometerTiltCompensatedY = magnetometerReadings.rawX * Math.Sin(roll) * Math.Sin(pitch) + magnetometerReadings.rawY * Math.Cos(roll) - magnetometerReadings.rawZ * Math.Sin(roll) * Math.Cos(pitch);
 
                // Calculate tilt compensated heading
                magnetometerReadings.headingInDegreesTiltCompensated = 180 * Math.Atan2(magnetometerTiltCompensatedY, magnetometerTiltCompensatedX) / Math.PI;
 
                if (magnetometerReadings.headingInDegreesTiltCompensated < 0)
                    magnetometerReadings.headingInDegreesTiltCompensated += 360;
 
                // Copy these values into their corresponding strings
                textGyroscopeRawX = String.Format("Gyro X Axis: {0:F0}", gyroscopeReadings.rawX);
                textGyroscopeRawY = String.Format("Gyro Y Axis: {0:F0}", gyroscopeReadings.rawY);
                textGyroscopeRawZ = String.Format("Gyro Z Axis: {0:F0}", gyroscopeReadings.rawZ);
                textAccelerometerRawX = String.Format("Acc X Axis: {0:F0}", accelerometerReadings.rawX);
                textAccelerometerRawY = String.Format("Acc Y Axis: {0:F0}", accelerometerReadings.rawY);
                textAccelerometerRawZ = String.Format("Acc Z Axis: {0:F0}", accelerometerReadings.rawZ);
                textMagnetometerRawX = String.Format("Mag X Axis: {0:F0}", magnetometerReadings.rawX);
                textMagnetometerRawY = String.Format("Mag Y Axis: {0:F0}", magnetometerReadings.rawY);
                textMagnetometerRawZ = String.Format("Mag Z Axis: {0:F0}", magnetometerReadings.rawZ);
                textGyroscopeAngleX = String.Format("Gyr X Angle: {0:F2}", gyroscopeAngleX);
                textGyroscopeAngleY = String.Format("Gyr Y Angle: {0:F2}", gyroscopeAngleY);
                textGyroscopeAngleZ = String.Format("Gyr Z Angle: {0:F2}", gyroscopeAngleZ);
                textAccelerometerangleX = String.Format("Acc X Angle: {0:F2}", accelerometerAngleInDegreesX);
                textAccelerometerAngleY = String.Format("Acc Y Angle: {0:F2}", accelerometerAngleInDegreesY);
                textFusedX = String.Format(" {0:F0}", complimentaryFilterFusedAngleX);
                textFusedY = String.Format(" {0:F0}", complimentaryFilterFusedAngleY);
                textHeading = String.Format(" {0:F0}", magnetometerReadings.headingInDegrees);
                textHeadingTiltCompensated = String.Format(" {0:F0}", magnetometerReadings.headingInDegreesTiltCompensated);
                textStatus = "Status: Running";
            }
            // If there are problems, display error message
            catch (Exception ex)
            {
                textGyroscopeRawX = "X Raw: Error";
                textGyroscopeRawY = "Y Raw: Error";
                textGyroscopeRawZ = "Z Raw: Error";
                textAccelerometerRawX = "X Raw: Error";
                textAccelerometerRawY = "Y Raw: Error";
                textAccelerometerRawZ = "Z Raw: Error";
                textAccelerometerangleX = " Acc X Angle: Error";
                textAccelerometerAngleY = " Acc Y Angle: Error";
                textGyroscopeAngleX = " Acc X Angle: Error";
                textGyroscopeAngleY = " Acc Y Angle: Error";
                textGyroscopeAngleZ = " Acc Z Angle: Error";
                textFusedX = " Fused X Error";
                textFusedY = " Fused Y Error";
                textHeading = " Error";
                textHeadingTiltCompensated = " Error";
 
                textStatus = "Failed to read from Gyroscope: " + ex.Message;
            }
 
            // UI updates must be invoked on the UI thread
            var task = this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                // Here we copy all the strings into their display elements
                DisplayTextGyrX.Text = textGyroscopeRawX;
                DisplayTextGyrY.Text = textGyroscopeRawY;
                DisplayTextGyrZ.Text = textGyroscopeRawZ;
 
                DisplayTextAccX.Text = textAccelerometerRawX;
                DisplayTextAccY.Text = textAccelerometerRawY;
                DisplayTextAccZ.Text = textAccelerometerRawZ;
 
                DisplayTextAccXAngle.Text = textAccelerometerangleX;
                DisplayTextAccYAngle.Text = textAccelerometerAngleY;
 
                DisplayTextGyrAngleX.Text = textGyroscopeAngleX;
                DisplayTextGyrAngleY.Text = textGyroscopeAngleY;
                DisplayTextGyrAngleZ.Text = textGyroscopeAngleZ;
 
                DisplayTextFusedXvalue.Text = textFusedX;
                DisplayTextFusedYvalue.Text = textFusedY;
 
                DisplayTextHeadingValue.Text = textHeading;
                DisplayTextHeadingTiltCompensatedValue.Text = textHeadingTiltCompensated;
 
                DisplayTextStatus.Text = textStatus;
            });
        }
 
        // Read a series of bytes from the gyroscope
        private byte[] ReadBytesFromGyroscope(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);  // The MSB is set as this is required by the LSM9DSO to auto increment when reading a series of bytes
            i2cDeviceGyroscope.WriteRead(buffer, values);
            return values;
        }
 
        // Read a series of bytes from the accelerometer
        private byte[] ReadBytesFromAccelerometer(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);
            i2cDeviceAccelerometerMagnetometer.WriteRead(buffer, values);
            return values;
        }
 
        // Read a series of bytes from the magnetometer
        private byte[] ReadBytesFromMagnetometer(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);
            i2cDeviceAccelerometerMagnetometer.WriteRead(buffer, values);    // The magnetometer uses the same I2C slave address as the accelerometer
            return values;
        }
 
        // Write a byte to the gyroscope
        private void WriteByteToGyroscope(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] writeBuf = new byte[] { regAddr, value };
            i2cDeviceGyroscope.Write(writeBuf);
        }
 
        // Write a byte to the accelerometer
        private void WriteByteToAccelerometer(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] writeBuf = new byte[] { regAddr, value };
            i2cDeviceAccelerometerMagnetometer.Write(writeBuf);
        }
 
        // Write a byte to the magnetometer
        private void WriteByteToMagnetometer(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] writeBuf = new byte[] { regAddr, value };
            i2cDeviceAccelerometerMagnetometer.Write(writeBuf);      // The magnetometer uses the same I2C slave address as the accelerometer
        }
 
        private Gyroscope GetGyroscopeReadings()
        {
            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value)
            //Start at OUT_X_L_G and read 6 bytes.
            byte[] data = ReadBytesFromGyroscope(LSM9DS0.OUT_X_L_G, (byte)6);
 
            int gyroscopeRawX = (int)(data[0] | (data[1] << 8));
            int gyroscopeRawY = (int)(data[2] | (data[3] << 8));
            int gyroscopeRawZ = (int)(data[4] | (data[5] << 8));
            if (gyroscopeRawX >= 32768) gyroscopeRawX = gyroscopeRawX - 65536;
            if (gyroscopeRawY >= 32768) gyroscopeRawY = gyroscopeRawY - 65536;
            if (gyroscopeRawZ >= 32768) gyroscopeRawZ = gyroscopeRawZ - 65536;
 
            Gyroscope gyroscopeReadings;
            gyroscopeReadings.rawX = gyroscopeRawX;
            gyroscopeReadings.rawY = gyroscopeRawY;
            gyroscopeReadings.rawZ = gyroscopeRawZ;
 
            return gyroscopeReadings;
        }
 
        private Accelerometer GetAccelerometerReadings()
        {
            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value)
            //Start at OUT_X_L_A and read 6 bytes.
 
            byte[] data = ReadBytesFromAccelerometer(LSM9DS0.OUT_X_L_A, (byte)6);
 
            int accelerometerRawX = (int)(data[0] | (data[1] << 8));
            int accelerometerRawY = (int)(data[2] | (data[3] << 8));
            int accelerometerRawZ = (int)(data[4] | (data[5] << 8));
            if (accelerometerRawX >= 32768) accelerometerRawX = accelerometerRawX - 65536;
            if (accelerometerRawY >= 32768) accelerometerRawY = accelerometerRawY - 65536;
            if (accelerometerRawZ >= 32768) accelerometerRawZ = accelerometerRawZ - 65536;
 
            Accelerometer accelerometerReadings;
            accelerometerReadings.rawX = accelerometerRawX;
            accelerometerReadings.rawY = accelerometerRawY;
            accelerometerReadings.rawZ = accelerometerRawZ;
 
            return accelerometerReadings;
        }
 
        private Magnetometer GetMagnetometerReadings()
        {
            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value)
            //Start at OUT_X_L_A and read 6 bytes.
 
            byte[] data = ReadBytesFromMagnetometer(LSM9DS0.OUT_X_L_M, (byte)6);
 
            int magnetometerRawX = (int)(data[0] | (data[1] << 8));
            int magnetometerRawY = (int)(data[2] | (data[3] << 8));
            int magnetometerRawZ = (int)(data[4] | (data[5] << 8));
            if (magnetometerRawX >= 32768) magnetometerRawX = magnetometerRawX - 65536;
            if (magnetometerRawY >= 32768) magnetometerRawY = magnetometerRawY - 65536;
            if (magnetometerRawZ >= 32768) magnetometerRawZ = magnetometerRawZ - 65536;
 
            Magnetometer magnetometerReadings;
            magnetometerReadings.rawX = magnetometerRawX;
            magnetometerReadings.rawY = magnetometerRawY;
            magnetometerReadings.rawZ = magnetometerRawZ;
            magnetometerReadings.headingInDegrees = 0.0;
            magnetometerReadings.headingInDegreesTiltCompensated = 0.0;
 
            return magnetometerReadings;
        }
    }
}
