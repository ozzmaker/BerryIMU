#define MAG_ADDRESS            (0x3A >> 1)
#define ACC_ADDRESS            (0x3A >> 1)
//#define ACC_ADDRESS_SA0_A_LOW  (0x30 >> 1)
//#define ACC_ADDRESS_SA0_A_HIGH (0x32 >> 1)



#define LSM303D_TEMP_OUT_L      0x05    // R
#define LSM303D_TEMP_OUT_H      0x06    // R
#define LSM303D_STATUS_M        0x07    // R
#define LSM303D_OUT_X_L_M       0x08    // R
#define LSM303D_OUT_X_H_M       0x09    // R
#define LSM303D_OUT_Y_L_M       0x0A    // R
#define LSM303D_OUT_Y_H_M       0x0B    // R
#define LSM303D_OUT_Z_L_M       0x0C    // R
#define LSM303D_OUT_Z_H_M       0x0D    // R
#define LSM303D_WHO_AM_I        0x0F    // R
#define LSM303D_CTRL0           0x1F    // RW
#define LSM303D_CTRL1           0x20    // RW
#define LSM303D_CTRL2           0x21    // RW
#define LSM303D_CTRL3           0x22    // RW
#define LSM303D_CTRL4           0x23    // RW
#define LSM303D_CTRL5           0x24    // RW
#define LSM303D_CTRL6           0x25    // RW
#define LSM303D_CTRL7           0x26    // RW
#define LSM303D_STATUS_A        0x27    // R
#define LSM303D_OUT_X_L_A       0x28    // R
#define LSM303D_OUT_X_H_A       0x29    // R
#define LSM303D_OUT_Y_L_A       0x2A    // R
#define LSM303D_OUT_Y_H_A       0x2B    // R
#define LSM303D_OUT_Z_L_A       0x2C    // R
#define LSM303D_OUT_Z_H_A       0x2D    // R
