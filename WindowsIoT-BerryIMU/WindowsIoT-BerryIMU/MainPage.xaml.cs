using System;
using System.Threading;
using Windows.UI.Xaml.Controls;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
using System.Text;

namespace BerryImu
{
 
    public sealed partial class MainPage : Page
    {
        private Timer periodicTimer;
        private baseLSM9DS lsm9Ds;

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

            try
            {
                var str = new StringBuilder();
                foreach (var device in discoveredI2cDevices)
                {
                    str.AppendLine($"{device.Name} ({device.Id}) {device.Kind}.");
                }
                DisplayTextStatus.Text = str.ToString();

                lsm9Ds = discoveredI2cDevices[0].Id[discoveredI2cDevices[0].Id.Length - 1] == '1' ?
                    (baseLSM9DS)new LSM9DS1() : (baseLSM9DS)new LSM9DS0();

                await lsm9Ds.Initialise(discoveredI2cDevices[0].Id);

                // Now that everything is initialized, create a timer so we read data every DT
                periodicTimer = new Timer(this.TimerCallback, null, 0, loopDeltaInMilliseconds);

                DisplayTextStatus.Text = "Initialized I2CBerryIMU";
            }
            catch (Exception ex)
            {
                DisplayTextStatus.Text = ex.ToString();
            }
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            // Cleanup
            lsm9Ds.Dispose();
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
                Gyroscope gyroscopeReadings = lsm9Ds.GetGyroscopeReadings();
 
                //Convert Gyro raw data to degrees/s
                gyroscopeDegreesPerSecondX = gyroscopeReadings.rawX * gyroscopeAngularRateSensitivity;
                gyroscopeDegreesPerSecondY = gyroscopeReadings.rawY * gyroscopeAngularRateSensitivity;
                gyroscopeDegreesPerSecondZ = gyroscopeReadings.rawZ * gyroscopeAngularRateSensitivity;
 
                //Calculate the angles from the Gyroscope
                gyroscopeAngleX += gyroscopeDegreesPerSecondX * loopDeltaInMilliseconds / 1000;
                gyroscopeAngleY += gyroscopeDegreesPerSecondY * loopDeltaInMilliseconds / 1000;
                gyroscopeAngleZ += gyroscopeDegreesPerSecondZ * loopDeltaInMilliseconds / 1000;
 
                Accelerometer accelerometerReadings = lsm9Ds.GetAccelerometerReadings();
 
                //Convert Accelerometer values to degrees
                var accelerometerAngleInDegreesX = (Math.Atan2(accelerometerReadings.rawY, accelerometerReadings.rawZ) + Math.PI) * (180.0 / Math.PI);
                var accelerometerAngleInDegreesY = (Math.Atan2(accelerometerReadings.rawZ, accelerometerReadings.rawX) + Math.PI) * (180.0 / Math.PI);
 
                Magnetometer magnetometerReadings = lsm9Ds.GetMagnetometerReadings();
 
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
     }
}
