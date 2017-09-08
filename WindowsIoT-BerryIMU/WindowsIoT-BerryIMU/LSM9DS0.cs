using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BerryImu
{
    class LSM9DS0
    {
        //I2C addresses
        public const byte MAG_ADDRESS = 0x1E;
        public const byte ACC_ADDRESS = 0x1E;
        public const byte GYR_ADDRESS = 0x6A;


        //////////////////////////////////////////
        /**LSM9DS0GyroRegisters**/

        public const byte WHO_AM_I_G = 0x0F;
        public const byte CTRL_REG1_G = 0x20;
        public const byte CTRL_REG2_G = 0x21;
        public const byte CTRL_REG3_G = 0x22;
        public const byte CTRL_REG4_G = 0x23;
        public const byte CTRL_REG5_G = 0x24;
        public const byte REFERENCE_G = 0x25;
        public const byte STATUS_REG_G = 0x27;
        public const byte OUT_X_L_G = 0x28;
        public const byte OUT_X_H_G = 0x29;
        public const byte OUT_Y_L_G = 0x2A;
        public const byte OUT_Y_H_G = 0x2B;
        public const byte OUT_Z_L_G = 0x2C;
        public const byte OUT_Z_H_G = 0x2D;
        public const byte FIFO_CTRL_REG_G = 0x2E;
        public const byte FIFO_SRC_REG_G = 0x2F;
        public const byte INT1_CFG_G = 0x30;
        public const byte INT1_SRC_G = 0x31;
        public const byte INT1_THS_XH_G = 0x32;
        public const byte INT1_THS_XL_G = 0x33;
        public const byte INT1_THS_YH_G = 0x34;
        public const byte INT1_THS_YL_G = 0x35;
        public const byte INT1_THS_ZH_G = 0x36;
        public const byte INT1_THS_ZL_G = 0x37;
        public const byte INT1_DURATION_G = 0x38;

        //////////////////////////////////////////
        //LSM9DS0Accel/Magneto(XM)Registers//
        //////////////////////////////////////////
        public const byte OUT_TEMP_L_XM = 0x05;
        public const byte OUT_TEMP_H_XM = 0x06;
        public const byte STATUS_REG_M = 0x07;
        public const byte OUT_X_L_M = 0x08;
        public const byte OUT_X_H_M = 0x09;
        public const byte OUT_Y_L_M = 0x0A;
        public const byte OUT_Y_H_M = 0x0B;
        public const byte OUT_Z_L_M = 0x0C;
        public const byte OUT_Z_H_M = 0x0D;
        public const byte WHO_AM_I_XM = 0x0F;
        public const byte INT_CTRL_REG_M = 0x12;
        public const byte INT_SRC_REG_M = 0x13;
        public const byte INT_THS_L_M = 0x14;
        public const byte INT_THS_H_M = 0x15;
        public const byte OFFSET_X_L_M = 0x16;
        public const byte OFFSET_X_H_M = 0x17;
        public const byte OFFSET_Y_L_M = 0x18;
        public const byte OFFSET_Y_H_M = 0x19;
        public const byte OFFSET_Z_L_M = 0x1A;
        public const byte OFFSET_Z_H_M = 0x1B;
        public const byte REFERENCE_X = 0x1C;
        public const byte REFERENCE_Y = 0x1D;
        public const byte REFERENCE_Z = 0x1E;
        public const byte CTRL_REG0_XM = 0x1F;
        public const byte CTRL_REG1_XM = 0x20;
        public const byte CTRL_REG2_XM = 0x21;
        public const byte CTRL_REG3_XM = 0x22;
        public const byte CTRL_REG4_XM = 0x23;
        public const byte CTRL_REG5_XM = 0x24;
        public const byte CTRL_REG6_XM = 0x25;
        public const byte CTRL_REG7_XM = 0x26;
        public const byte STATUS_REG_A = 0x27;
        public const byte OUT_X_L_A = 0x28;
        public const byte OUT_X_H_A = 0x29;
        public const byte OUT_Y_L_A = 0x2A;
        public const byte OUT_Y_H_A = 0x2B;
        public const byte OUT_Z_L_A = 0x2C;
        public const byte OUT_Z_H_A = 0x2D;
        public const byte FIFO_CTRL_REG = 0x2E;
        public const byte FIFO_SRC_REG = 0x2F;
        public const byte INT_GEN_1_REG = 0x30;
        public const byte INT_GEN_1_SRC = 0x31;
        public const byte INT_GEN_1_THS = 0x32;
        public const byte INT_GEN_1_DURATION = 0x33;
        public const byte INT_GEN_2_REG = 0x34;
        public const byte INT_GEN_2_SRC = 0x35;
        public const byte INT_GEN_2_THS = 0x36;
        public const byte INT_GEN_2_DURATION = 0x37;
        public const byte CLICK_CFG = 0x38;
        public const byte CLICK_SRC = 0x39;
        public const byte CLICK_THS = 0x3A;
        public const byte TIME_LIMIT = 0x3B;
        public const byte TIME_LATENCY = 0x3C;
        public const byte TIME_WINDOW = 0x3D;



    }
}
