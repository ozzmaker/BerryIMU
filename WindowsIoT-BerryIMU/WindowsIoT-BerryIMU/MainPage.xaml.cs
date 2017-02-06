/*
    BerryIMU -   Ozzmaker.com
*/

using System;
using System.Threading;
using Windows.UI.Xaml.Controls;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;


namespace BERRYIMU
{
    struct Gyroscope
    {
        public double X;
        public double Y;
        public double Z;
     };
    struct Accelerometer
    {
        public double X;
        public double Y;
        public double Z;
    };
    struct Magnetometer
    {
        public double X;
        public double Y;
        public double Z;
        public double heading;
        public double headingTiltCompensated;
    };



    public sealed partial class MainPage : Page
	{
		
		private I2cDevice I2CGYR, I2CACC;
		private Timer periodicTimer;
        float RAD_TO_DEG = 57.29578f;
        const float G_GAIN = 0.070f;  // [deg/s/LSB]  If you change the dps for gyro, you need to update this value accordingly
        const float AA = 0.03f;     // Complementary filter constant
        const int DT = 100;         //DT is the loop delta in milliseconds.
        float gyroXangle = 0.0f;
        float gyroYangle = 0.0f;
        float gyroZangle = 0.0f;
        bool IMU_upside_down = true;		//set to false if BerryIMU is up the correct way. This is when the skull logo is facing down.
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
			string aqs = I2cDevice.GetDeviceSelector();		        // Get a selector string that will return all I2C controllers on the system
			var dis = await DeviceInformation.FindAllAsync(aqs);	// Find the I2C bus controller device with our selector string
			if (dis.Count == 0)
			{
                DisplayText_Status.Text = "No I2C controllers were found on the system";
				return;
			}

            // Specify I2C slave addresses for the Gyro and Accelerometer on BerryIMU.  
            //      Note; the magenetormeter(compass) uses the same address as the accelerometer. 
            //      We will use the ACC I2C settings to access both the accelerometer and the magnetometer.
			var settingsGYR = new I2cConnectionSettings(berrryIMU.GYR_ADDRESS);
            var settingsACC = new I2cConnectionSettings(berrryIMU.ACC_ADDRESS);
            

            // Enable 400kHz I2C buss
            settingsGYR.BusSpeed = I2cBusSpeed.FastMode;
            settingsACC.BusSpeed = I2cBusSpeed.FastMode;
            

            // Create an I2cDevices with our selected bus controller and I2C settings
            I2CGYR = await I2cDevice.FromIdAsync(dis[0].Id, settingsGYR);	
            I2CACC = await I2cDevice.FromIdAsync(dis[0].Id, settingsACC);




            // Enable the gyrscope
            writeByteGYR(berrryIMU.CTRL_REG1_G, 0x0F);    // Normal power mode, all axes enabled)
            writeByteGYR(berrryIMU.CTRL_REG4_G, 0x30);    // Continuos update, 2000 dps full scale
            
            //Enable the accelerometer
            writeByteACC(berrryIMU.CTRL_REG1_XM, 0x67);    // z,y,x axis enabled, continuous update,  100Hz data rate
            writeByteACC(berrryIMU.CTRL_REG2_XM, 0x20);    // +/- 16G full scale

            // Enable the magnetometer
            writeByteMAG(berrryIMU.CTRL_REG5_XM, 0xF0); // Temp enable, M data rate = 50Hz
            writeByteMAG(berrryIMU.CTRL_REG6_XM, 0x60); // +/-12gauss
            writeByteMAG(berrryIMU.CTRL_REG7_XM, 0x0);  // Continuous - conversion mode


            
            // Now that everything is initialized, create a timer so we read data every DT
            periodicTimer = new Timer(this.TimerCallback, null, 0,DT);// );
		}

		private void MainPage_Unloaded(object sender, object args)
		{
			// Cleanup
			I2CGYR.Dispose();
            I2CACC.Dispose();

        }

		private void TimerCallback(object state)
		{
            // These strings are used to store the readings from the sensors. 
            string textGyrX, textGyrY, textGyrZ;
            string textAccX, textAccY, textAccZ;
            string textMagX, textMagY, textMagZ;
            string textGyrAngleX, textGyrAngleY, textGyrAngleZ;
            string textAccXangle, textAccYangle;
            String textHeading, textHeadingTiltCompensated;
            string textFusedX, textFusedY;
            string addressText, statusText;


            
            float CFangleX = 0.0f;      // Fused X angle
            float CFangleY = 0.0f;      // Fused Y angle
            float rate_gyr_x = 0.0f;    // [deg/s]
            float rate_gyr_y = 0.0f;    // [deg/s]
            float rate_gyr_z = 0.0f;    // [deg/s]

            




            try
            {
                // read the raw values from the gyrscope
                Gyroscope gyr = ReadI2C_GYR();
                // Copy these values into their corresponding strings
                textGyrX = String.Format("Gyro X Axis: {0:F0}", gyr.X);
				textGyrY = String.Format("Gyro Y Axis: {0:F0}", gyr.Y);
                textGyrZ = String.Format("Gyro Z Axis: {0:F0}", gyr.Z);
                
                // Read the raw values from the accelerometer
                Accelerometer acc = ReadI2C_ACC();
                textAccX = String.Format("Acc X Axis: {0:F0}", acc.X);
                textAccY = String.Format("Acc Y Axis: {0:F0}", acc.Y);
                textAccZ = String.Format("Acc Z Axis: {0:F0}", acc.Z);
                // Read the raw values from the magnetometer
                Magnetometer mag = ReadI2C_MAG();
                textMagX = String.Format("Mag X Axis: {0:F0}", mag.X);
                textMagY = String.Format("Mag Y Axis: {0:F0}", mag.Y);
                textMagZ = String.Format("Mag Z Axis: {0:F0}", mag.Z);
                


                statusText = "Status: Running";

                //Convert Gyro raw to degrees per second
                rate_gyr_x = (float)gyr.X * G_GAIN;
                rate_gyr_y = (float)gyr.Y * G_GAIN;
                rate_gyr_z = (float)gyr.Z * G_GAIN;

                //Calculate the angles from the gyro
                gyroXangle += rate_gyr_x * DT / 1000;
                gyroYangle += rate_gyr_y * DT / 1000;
                gyroZangle += rate_gyr_z * DT / 1000;

                // Copy these values into their corresponding strings
                textGyrAngleX = String.Format("Gyr X Angle: {0:F2}", gyroXangle);
                textGyrAngleY = String.Format("Gyr Y Angle: {0:F2}", gyroYangle);
                textGyrAngleZ = String.Format("Gyr Z Angle: {0:F2}", gyroZangle);
                
                //Convert Accelerometer values to degrees
                float AccXangle = (float)(Math.Atan2(acc.Y, acc.Z) + Math.PI) * RAD_TO_DEG;
                float AccYangle = (float)(Math.Atan2(acc.Z, acc.X) + Math.PI) * RAD_TO_DEG;
                // Copy these values into their corresponding strings
                textAccXangle = String.Format("Acc X Angle: {0:F2}", AccXangle);
                textAccYangle = String.Format("Acc Y Angle: {0:F2}", AccYangle);


                if (IMU_upside_down) {
                    if (AccXangle > 180)
                        AccXangle -= (float)360.0;
                    AccYangle -= 90;
                    if (AccYangle > 180)
                        AccYangle -= (float)360.0;
                    //Only needed if the heading value does not increase when the magnetometer is rotated clockwise
                    mag.Y = -mag.Y;
                }
                else {
                    AccXangle -= (float)180.0;
                    if (AccYangle > 90)
                        AccYangle -= (float)270;
                    else
                        AccYangle += (float)90; 
                }    


                

                //Complementary filter used to combine the accelerometer and gyro values.
                CFangleX = AA * (CFangleX + (rate_gyr_x * DT / 1000)) + (1.0f - AA) * AccXangle;
                CFangleY = AA * (CFangleY + (rate_gyr_y * DT / 1000)) + (1.0f - AA) * AccYangle;
                // Copy these values into their corresponding strings
                textFusedX = String.Format("{0:F0}", CFangleX);
                textFusedY = String.Format("{0:F0}", CFangleY);



                mag.heading = (float)(180f * Math.Atan2(mag.Y, mag.X) / Math.PI);
               
                // Have our heading between 0 and 360
	            if (mag.heading < 0)
	 	            mag.heading += 360;
                textHeading = String.Format("{0:F0}", mag.heading);



                // Tilt compensated heading calculations
                //
                // Normalize accelerometer raw values.
                double accXnorm = acc.X / Math.Sqrt(acc.X * acc.X + acc.Y * acc.Y + acc.Z * acc.Z);
                double accYnorm = acc.Y / Math.Sqrt(acc.X * acc.X + acc.Y * acc.Y + acc.Z * acc.Z);



                //////////////////////////// Calculate pitch and roll//////////////////////////
                if (IMU_upside_down)
                {
                    // Use these four lines when the IMU is upside down. Skull logo is facing up
                    accXnorm = -accXnorm;               //flip Xnorm as the IMU is upside down
                    accYnorm = -accYnorm;               //flip Ynorm as the IMU is upside down
                    pitch = Math.Asin(accXnorm);
                    roll = Math.Asin(accYnorm / Math.Cos(pitch));
                }
                else
                {
                    // Us these two lines when the IMU is up the right way. Skull logo is facing down
                    pitch = Math.Asin(accXnorm);
                    roll = -Math.Asin(accYnorm / Math.Cos(pitch));
                }


                // Calculate the new tilt compensated values
                double magXcomp = mag.X * Math.Cos(pitch) + mag.Z * Math.Sin(pitch);
                double magYcomp = mag.X * Math.Sin(roll) * Math.Sin(pitch) + mag.Y * Math.Cos(roll) - mag.Z * Math.Sin(roll) * Math.Cos(pitch);

                // Calculate tilt compensated heading
                mag.headingTiltCompensated = 180 * Math.Atan2(magYcomp, magXcomp) / Math.PI;

                if (mag.headingTiltCompensated < 0)
                    mag.headingTiltCompensated += 360;

                // Copy the tilt compensated heading into its corresponding string
                textHeadingTiltCompensated = String.Format("{0:F0}", mag.headingTiltCompensated);
            }
            // If there are problems, display error message
            catch (Exception ex)
			{
                textGyrX = "X Raw: Error";
				textGyrY = "Y Raw: Error";
                textGyrZ = "Z Raw: Error";
                textAccX = "X Raw: Error";
                textAccY = "Y Raw: Error";
                textAccZ = "Z Raw: Error";
                textAccXangle = " Acc X Angle: Error";
                textAccYangle = " Acc Y Angle: Error";
                textGyrAngleX = " Acc X Angle: Error";
                textGyrAngleY = " Acc Y Angle: Error";
                textGyrAngleZ = " Acc Z Angle: Error";
                textFusedX = " Fused X Error";
                textFusedY = "Fused Y Error";
                textHeading = "Error";
                textHeadingTiltCompensated = "Error";

                statusText = "Failed to read from Gyroscope: " + ex.Message;
			}

			// UI updates must be invoked on the UI thread
			var task = this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
			{
                // Here we copy all the strings into their display elements
                DisplayTextGyrX.Text = textGyrX;
                DisplayTextGyrY.Text = textGyrY;
                DisplayTextGyrZ.Text = textGyrZ;

                DisplayTextAccX.Text = textAccX;
                DisplayTextAccY.Text = textAccY;
                DisplayTextAccZ.Text = textAccZ;

                DisplayTextAccXAngle.Text = textAccXangle;
                DisplayTextAccYAngle.Text = textAccYangle;

                DisplayTextGyrAngleX.Text = textGyrAngleX;
                DisplayTextGyrAngleY.Text = textGyrAngleY;
                DisplayTextGyrAngleZ.Text = textGyrAngleZ;

                DisplayTextFusedXvalue.Text = textFusedX;
                DisplayTextFusedYvalue.Text = textFusedY;
                
                DisplayTextHeadingValue.Text = textHeading;
                DisplayTextHeadingTiltCompensatedValue.Text = textHeadingTiltCompensated;

                DisplayText_Status.Text = statusText;
            });
		}

        // Read a series of bytes from the gyroscope 
        private byte[] readBytesGYR(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);  // The MSB is set as this is required by the LSM9DSO to auto increment when reading a series of bytes
            I2CGYR.WriteRead(buffer, values);
            return values;
        }
        // Read a series of bytes from the accelerometer 
        private byte[] readBytesACC(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);
            I2CACC.WriteRead(buffer, values);
            return values;
        }
        // Read a series of bytes from the magnetometer 
        private byte[] readBytesMAG(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);
            I2CACC.WriteRead(buffer, values);    // The magnetometer uses the same I2C slave address as the accelerometer
            return values;
        }
        // Write a byte to the gyroscope
        private void writeByteGYR(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] WriteBuf = new byte[] { regAddr, value };
            I2CGYR.Write(WriteBuf);   
         }
        // Write a byte to the accelerometer
        private void writeByteACC(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] WriteBuf = new byte[] { regAddr, value };
            I2CACC.Write(WriteBuf);     
        }
        // Write a byte to the magnetometer
        private void writeByteMAG(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] WriteBuf = new byte[] { regAddr, value };
            I2CACC.Write(WriteBuf);      // The magnetometer uses the same I2C slave address as the accelerometer
        }







        private Gyroscope ReadI2C_GYR()
		{

            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value) 
            //Start at OUT_X_L_G and read 6 bytes.

            byte[] data = readBytesGYR(berrryIMU.OUT_X_L_G, (byte)6);

            int gyrRawX = (int)(data[0] | (data[1] << 8));
            int gyrRawY = (int)(data[2] | (data[3] << 8));
            int gyrRawZ = (int)(data[4] | (data[5] << 8));
            if (gyrRawX >= 32768) gyrRawX = gyrRawX - 65536;
            if (gyrRawY >= 32768) gyrRawY = gyrRawY - 65536;
            if (gyrRawZ >= 32768) gyrRawZ = gyrRawZ - 65536;

                
            Gyroscope gyro;
			gyro.X = gyrRawX;
			gyro.Y = gyrRawY;
			gyro.Z = gyrRawZ;
            
            
            return gyro;
		}



        private Accelerometer ReadI2C_ACC()
        {

            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value) 
            //Start at OUT_X_L_A and read 6 bytes.

            byte[] data = readBytesACC(berrryIMU.OUT_X_L_A, (byte)6);

            int accRawX = (int)(data[0] | (data[1] << 8));
            int accRawY = (int)(data[2] | (data[3] << 8));
            int accRawZ = (int)(data[4] | (data[5] << 8));
            if (accRawX >= 32768) accRawX = accRawX - 65536;
            if (accRawY >= 32768) accRawY = accRawY - 65536;
            if (accRawZ >= 32768) accRawZ = accRawZ - 65536;

            
            Accelerometer acc;
            acc.X = accRawX;
            acc.Y = accRawY;
            acc.Z = accRawZ;

            return acc;

        }
        private Magnetometer ReadI2C_MAG()
        {

            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value) 
            //Start at OUT_X_L_A and read 6 bytes.

            byte[] data = readBytesMAG(berrryIMU.OUT_X_L_M, (byte)6);

            int accMagX = (int)(data[0] | (data[1] << 8));
            int accMagY = (int)(data[2] | (data[3] << 8));
            int accMagZ = (int)(data[4] | (data[5] << 8));
            if (accMagX >= 32768) accMagX = accMagX - 65536;
            if (accMagY >= 32768) accMagY = accMagY - 65536;
            if (accMagZ >= 32768) accMagZ = accMagZ - 65536;


            Magnetometer mag = new Magnetometer() ;
            mag.X = accMagX;
            mag.Y = accMagY;
            mag.Z = accMagZ;
            
            return mag;

        }
    }
}
