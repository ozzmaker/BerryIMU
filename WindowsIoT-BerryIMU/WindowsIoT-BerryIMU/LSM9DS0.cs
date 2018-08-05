using System;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace BerryImu
{
    /// <summary>
    /// For BerryIMUv1
    /// </summary>
    internal class LSM9DS0: baseLSM9DS
    {
        //I2C addresses (0)
        public override byte MAG_ADDRESS => 0x1E;
        public override byte ACC_ADDRESS => 0x1E;
        public override byte GYR_ADDRESS => 0x6A;
        public override byte GYR_OUT_X_L_G => OUT_X_L_G;
        public override byte ACC_OUT_X_L => OUT_X_L_A;
        public override byte MAG_OUT_X_L => OUT_X_L_M;


        public override async Task Initialise(string discoveredI2cDevice)
        {
            // Specify I2C slave addresses for the Gyro and Accelerometer on BerryIMU. 
            //      Note; the magenetormeter(compass) uses the same address as the accelerometer.
            //      We will use the ACC_ADDRESS I2C settings to access both the accelerometer and the magnetometer.
            var i2cConnectionSettingsGyroscope = new I2cConnectionSettings(GYR_ADDRESS);
            var i2cConnectionSettingsAccelerometerMagnetometer = new I2cConnectionSettings(ACC_ADDRESS);

            // Enable 400kHz I2C bus speed
            i2cConnectionSettingsGyroscope.BusSpeed = I2cBusSpeed.FastMode; // 400kHz
            i2cConnectionSettingsAccelerometerMagnetometer.BusSpeed = I2cBusSpeed.FastMode; // 400kHz

            // Create I2cDevices with our selected bus controller and I2C settings
            i2cDeviceGyroscope = await I2cDevice.FromIdAsync(discoveredI2cDevice, i2cConnectionSettingsGyroscope);

            var i2cDeviceAccelerometerMagnetometer = 
                await I2cDevice.FromIdAsync(discoveredI2cDevice, i2cConnectionSettingsAccelerometerMagnetometer);
            i2cDeviceAccelerometer = i2cDeviceMagnetometer = i2cDeviceAccelerometerMagnetometer;

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
        }

        //////////////////////////////////////////
        /**LSM9DS0GyroRegisters**/

        private const byte WHO_AM_I_G = 0x0F;
        private const byte CTRL_REG1_G = 0x20;
        private const byte CTRL_REG2_G = 0x21;
        private const byte CTRL_REG3_G = 0x22;
        private const byte CTRL_REG4_G = 0x23;
        private const byte CTRL_REG5_G = 0x24;
        private const byte REFERENCE_G = 0x25;
        private const byte STATUS_REG_G = 0x27;
        private const byte OUT_X_L_G = 0x28;
        private const byte OUT_X_H_G = 0x29;
        private const byte OUT_Y_L_G = 0x2A;
        private const byte OUT_Y_H_G = 0x2B;
        private const byte OUT_Z_L_G = 0x2C;
        private const byte OUT_Z_H_G = 0x2D;
        private const byte FIFO_CTRL_REG_G = 0x2E;
        private const byte FIFO_SRC_REG_G = 0x2F;
        private const byte INT1_CFG_G = 0x30;
        private const byte INT1_SRC_G = 0x31;
        private const byte INT1_THS_XH_G = 0x32;
        private const byte INT1_THS_XL_G = 0x33;
        private const byte INT1_THS_YH_G = 0x34;
        private const byte INT1_THS_YL_G = 0x35;
        private const byte INT1_THS_ZH_G = 0x36;
        private const byte INT1_THS_ZL_G = 0x37;
        private const byte INT1_DURATION_G = 0x38;

        //////////////////////////////////////////
        //LSM9DS0Accel/Magneto(XM)Registers//
        //////////////////////////////////////////
        private const byte OUT_TEMP_L_XM = 0x05;
        private const byte OUT_TEMP_H_XM = 0x06;
        private const byte STATUS_REG_M = 0x07;
        private const byte OUT_X_L_M = 0x08;
        private const byte OUT_X_H_M = 0x09;
        private const byte OUT_Y_L_M = 0x0A;
        private const byte OUT_Y_H_M = 0x0B;
        private const byte OUT_Z_L_M = 0x0C;
        private const byte OUT_Z_H_M = 0x0D;
        private const byte WHO_AM_I_XM = 0x0F;
        private const byte INT_CTRL_REG_M = 0x12;
        private const byte INT_SRC_REG_M = 0x13;
        private const byte INT_THS_L_M = 0x14;
        private const byte INT_THS_H_M = 0x15;
        private const byte OFFSET_X_L_M = 0x16;
        private const byte OFFSET_X_H_M = 0x17;
        private const byte OFFSET_Y_L_M = 0x18;
        private const byte OFFSET_Y_H_M = 0x19;
        private const byte OFFSET_Z_L_M = 0x1A;
        private const byte OFFSET_Z_H_M = 0x1B;
        private const byte REFERENCE_X = 0x1C;
        private const byte REFERENCE_Y = 0x1D;
        private const byte REFERENCE_Z = 0x1E;
        private const byte CTRL_REG0_XM = 0x1F;
        private const byte CTRL_REG1_XM = 0x20;
        private const byte CTRL_REG2_XM = 0x21;
        private const byte CTRL_REG3_XM = 0x22;
        private const byte CTRL_REG4_XM = 0x23;
        private const byte CTRL_REG5_XM = 0x24;
        private const byte CTRL_REG6_XM = 0x25;
        private const byte CTRL_REG7_XM = 0x26;
        private const byte STATUS_REG_A = 0x27;
        private const byte OUT_X_L_A = 0x28;
        private const byte OUT_X_H_A = 0x29;
        private const byte OUT_Y_L_A = 0x2A;
        private const byte OUT_Y_H_A = 0x2B;
        private const byte OUT_Z_L_A = 0x2C;
        private const byte OUT_Z_H_A = 0x2D;
        private const byte FIFO_CTRL_REG = 0x2E;
        private const byte FIFO_SRC_REG = 0x2F;
        private const byte INT_GEN_1_REG = 0x30;
        private const byte INT_GEN_1_SRC = 0x31;
        private const byte INT_GEN_1_THS = 0x32;
        private const byte INT_GEN_1_DURATION = 0x33;
        private const byte INT_GEN_2_REG = 0x34;
        private const byte INT_GEN_2_SRC = 0x35;
        private const byte INT_GEN_2_THS = 0x36;
        private const byte INT_GEN_2_DURATION = 0x37;
        private const byte CLICK_CFG = 0x38;
        private const byte CLICK_SRC = 0x39;
        private const byte CLICK_THS = 0x3A;
        private const byte TIME_LIMIT = 0x3B;
        private const byte TIME_LATENCY = 0x3C;
        private const byte TIME_WINDOW = 0x3D;

    }
}
