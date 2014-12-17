#ifndef LSM303_h
#define LSM303_h

#include <stdint.h>

// register addresses
#define MAG_ADDRESS            (0x3C >> 1)
#define ACC_ADDRESS            (0x32 >> 1)
#define ACC_ADDRESS_SA0_A_LOW  (0x30 >> 1)
#define ACC_ADDRESS_SA0_A_HIGH (0x32 >> 1)

#define LSM303_CTRL_REG1_A       0x20
#define LSM303_CTRL_REG2_A       0x21
#define LSM303_CTRL_REG3_A       0x22
#define LSM303_CTRL_REG4_A       0x23
#define LSM303_CTRL_REG5_A       0x24
#define LSM303_CTRL_REG6_A       0x25 // DLHC only
#define LSM303_HP_FILTER_RESET_A 0x25 // DLH, DLM only
#define LSM303_REFERENCE_A       0x26
#define LSM303_STATUS_REG_A      0x27

#define LSM303_OUT_X_L_A         0x28
#define LSM303_OUT_X_H_A         0x29
#define LSM303_OUT_Y_L_A         0x2A
#define LSM303_OUT_Y_H_A         0x2B
#define LSM303_OUT_Z_L_A         0x2C
#define LSM303_OUT_Z_H_A         0x2D

#define LSM303_FIFO_CTRL_REG_A   0x2E // DLHC only
#define LSM303_FIFO_SRC_REG_A    0x2F // DLHC only

#define LSM303_INT1_CFG_A        0x30
#define LSM303_INT1_SRC_A        0x31
#define LSM303_INT1_THS_A        0x32
#define LSM303_INT1_DURATION_A   0x33
#define LSM303_INT2_CFG_A        0x34
#define LSM303_INT2_SRC_A        0x35
#define LSM303_INT2_THS_A        0x36
#define LSM303_INT2_DURATION_A   0x37

#define LSM303_CLICK_CFG_A       0x38 // DLHC only
#define LSM303_CLICK_SRC_A       0x39 // DLHC only
#define LSM303_CLICK_THS_A       0x3A // DLHC only
#define LSM303_TIME_LIMIT_A      0x3B // DLHC only
#define LSM303_TIME_LATENCY_A    0x3C // DLHC only
#define LSM303_TIME_WINDOW_A     0x3D // DLHC only

#define LSM303_CRA_REG_M         0x00
#define LSM303_CRB_REG_M         0x01
#define LSM303_MR_REG_M          0x02

#define LSM303_OUT_X_H_M         0x03
#define LSM303_OUT_X_L_M         0x04
#define LSM303_OUT_Y_H_M         -1   // The addresses of the Y and Z magnetometer output registers 
#define LSM303_OUT_Y_L_M         -2   // are reversed on the DLM and DLHC relative to the DLH.
#define LSM303_OUT_Z_H_M         -3   // These four defines have dummy values so the library can 
#define LSM303_OUT_Z_L_M         -4   // determine the correct address based on the device type.

#define LSM303_SR_REG_M          0x09
#define LSM303_IRA_REG_M         0x0A
#define LSM303_IRB_REG_M         0x0B
#define LSM303_IRC_REG_M         0x0C

#define LSM303_WHO_AM_I_M        0x0F // DLM only

#define LSM303_TEMP_OUT_H_M      0x31 // DLHC only
#define LSM303_TEMP_OUT_L_M      0x32 // DLHC only
#define LSM303DLH_OUT_Y_H_M      0x05
#define LSM303DLH_OUT_Y_L_M      0x06
#define LSM303DLH_OUT_Z_H_M      0x07
#define LSM303DLH_OUT_Z_L_M      0x08

#define LSM303DLM_OUT_Z_H_M      0x05
#define LSM303DLM_OUT_Z_L_M      0x06
#define LSM303DLM_OUT_Y_H_M      0x07
#define LSM303DLM_OUT_Y_L_M      0x08

#define LSM303DLHC_OUT_Z_H_M     0x05
#define LSM303DLHC_OUT_Z_L_M     0x06


#endif
