using System;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace BerryImu
{
    /// <summary>
    /// BerryIMUv2
    /// </summary>
    internal class LSM9DS1 : baseLSM9DS
    {
        //I2C addresses (1)
        public override byte MAG_ADDRESS => 0x1C;   //Would be 0x1E if SDO_M is HIGH	
        public override byte ACC_ADDRESS => 0x6A;
        public override byte GYR_ADDRESS => 0x6A;   //Would be 0x6B if SDO_AG is HIGH
        public override byte GYR_OUT_X_L_G => OUT_X_L_G;
        public override byte ACC_OUT_X_L => OUT_X_L_XL;
        public override byte MAG_OUT_X_L => OUT_X_L_M;

        public override async Task Initialise(string discoveredI2cDevice)
        {
            // Specify I2C slave addresses for the Gyro and Accelerometer on BerryIMU. 
            //      Note; the Gyro uses the same address as the accelerometer.
            var i2cConnectionSettingsAccelerometerGyroscope = new I2cConnectionSettings(GYR_ADDRESS);
            var i2cConnectionSettingsMagnetometer = new I2cConnectionSettings(MAG_ADDRESS);

            // Enable 400kHz I2C bus speed
            i2cConnectionSettingsAccelerometerGyroscope.BusSpeed = I2cBusSpeed.FastMode; // 400kHz
            i2cConnectionSettingsMagnetometer.BusSpeed = I2cBusSpeed.FastMode; // 400kHz

            // Create I2cDevices with our selected bus controller and I2C settings
            var i2cDeviceAccelerometerGyroscope = 
                await I2cDevice.FromIdAsync(discoveredI2cDevice, i2cConnectionSettingsAccelerometerGyroscope);
            i2cDeviceAccelerometer = i2cDeviceGyroscope = i2cDeviceAccelerometerGyroscope;

            i2cDeviceMagnetometer =
                await I2cDevice.FromIdAsync(discoveredI2cDevice, i2cConnectionSettingsMagnetometer);

            // Enable the gyrscope
            WriteByteToGyroscope(CTRL_REG4, 0b00111000);    // z, y, x axis enabled for gyro
            WriteByteToGyroscope(CTRL_REG1_G, 0b10111000);    // Gyro ODR = 476Hz, 2000 dps
            WriteByteToGyroscope(ORIENT_CFG_G, 0b00111000);    // Gyro ODR = 476Hz, 2000 dps

            //Enable the accelerometer
            WriteByteToAccelerometer(CTRL_REG5_XL, 0b00111000);    // z,y,x axis enabled, continuous update,  100Hz data rate
            WriteByteToAccelerometer(CTRL_REG6_XL, 0b00101000);    // +/- 16G full scale

            // Enable the magnetometer
            WriteByteToMagnetometer(CTRL_REG1_M, 0b10011100); // Temp compensation enabled,Low power mode mode,80Hz ODR
            WriteByteToMagnetometer(CTRL_REG2_M, 0b01000000); // +/-12 gauss full scale
            WriteByteToMagnetometer(CTRL_REG3_M, 0b00000000);  // Continuous - conversion mode
            WriteByteToMagnetometer(CTRL_REG4_M, 0b00000000);  // Lower power mode for Z axis
        }

        /////////////////////////////////////////
        // LSM9DS1 Accel/Gyro (XL/G) Registers //
        /////////////////////////////////////////

        private const byte ACT_THS = 0x04;
        private const byte ACT_DUR = 0x05;
        private const byte INT_GEN_CFG_XL = 0x06;
        private const byte INT_GEN_THS_X_XL = 0x07;
        private const byte INT_GEN_THS_Y_XL = 0x08;
        private const byte INT_GEN_THS_Z_XL = 0x09;
        private const byte INT_GEN_DUR_XL = 0x0A;
        private const byte REFERENCE_G = 0x0B;
        private const byte INT1_CTRL = 0x0C;
        private const byte INT2_CTRL = 0x0D;
        private const byte WHO_AM_I_XG = 0x0F;
        private const byte CTRL_REG1_G = 0x10;
        private const byte CTRL_REG2_G = 0x11;
        private const byte CTRL_REG3_G = 0x12;
        private const byte ORIENT_CFG_G = 0x13;
        private const byte INT_GEN_SRC_G = 0x14;
        private const byte OUT_TEMP_L = 0x15;
        private const byte OUT_TEMP_H = 0x16;
        private const byte STATUS_REG_0 = 0x17;
        private const byte OUT_X_L_G = 0x18;
        private const byte OUT_X_H_G = 0x19;
        private const byte OUT_Y_L_G = 0x1A;
        private const byte OUT_Y_H_G = 0x1B;
        private const byte OUT_Z_L_G = 0x1C;
        private const byte OUT_Z_H_G = 0x1D;
        private const byte CTRL_REG4 = 0x1E;
        private const byte CTRL_REG5_XL = 0x1F;
        private const byte CTRL_REG6_XL = 0x20;
        private const byte CTRL_REG7_XL = 0x21;
        private const byte CTRL_REG8 = 0x22;
        private const byte CTRL_REG9 = 0x23;
        private const byte CTRL_REG10 = 0x24;
        private const byte INT_GEN_SRC_XL = 0x26;
        private const byte STATUS_REG_1 = 0x27;
        private const byte OUT_X_L_XL = 0x28;
        private const byte OUT_X_H_XL = 0x29;
        private const byte OUT_Y_L_XL = 0x2A;
        private const byte OUT_Y_H_XL = 0x2B;
        private const byte OUT_Z_L_XL = 0x2C;
        private const byte OUT_Z_H_XL = 0x2D;
        private const byte FIFO_CTRL = 0x2E;
        private const byte FIFO_SRC = 0x2F;
        private const byte INT_GEN_CFG_G = 0x30;
        private const byte INT_GEN_THS_XH_G = 0x31;
        private const byte INT_GEN_THS_XL_G = 0x32;
        private const byte INT_GEN_THS_YH_G = 0x33;
        private const byte INT_GEN_THS_YL_G = 0x34;
        private const byte INT_GEN_THS_ZH_G = 0x35;
        private const byte INT_GEN_THS_ZL_G = 0x36;
        private const byte INT_GEN_DUR_G = 0x37;


        ///////////////////////////////
        // LSM9DS1 Magneto Registers //
        ///////////////////////////////

        private const byte OFFSET_X_REG_L_M = 0x05;
        private const byte OFFSET_X_REG_H_M = 0x06;
        private const byte OFFSET_Y_REG_L_M = 0x07;
        private const byte OFFSET_Y_REG_H_M = 0x08;
        private const byte OFFSET_Z_REG_L_M = 0x09;
        private const byte OFFSET_Z_REG_H_M = 0x0A;
        private const byte WHO_AM_I_M = 0x0F;
        private const byte CTRL_REG1_M = 0x20;
        private const byte CTRL_REG2_M = 0x21;
        private const byte CTRL_REG3_M = 0x22;
        private const byte CTRL_REG4_M = 0x23;
        private const byte CTRL_REG5_M = 0x24;
        private const byte STATUS_REG_M = 0x27;
        private const byte OUT_X_L_M = 0x28;
        private const byte OUT_X_H_M = 0x29;
        private const byte OUT_Y_L_M = 0x2A;
        private const byte OUT_Y_H_M = 0x2B;
        private const byte OUT_Z_L_M = 0x2C;
        private const byte OUT_Z_H_M = 0x2D;
        private const byte INT_CFG_M = 0x30;
        private const byte INT_SRC_M = 0x30;
        private const byte INT_THS_L_M = 0x32;
        private const byte INT_THS_H_M = 0x33;

        ////////////////////////////////
        // LSM9DS1 WHO_AM_I Responses //
        ////////////////////////////////

        private const byte WHO_AM_I_AG_RSP = 0x68;
        private const byte WHO_AM_I_M_RSP = 0x3D;


    }
}
