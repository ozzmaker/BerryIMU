import smbus
bus = smbus.SMBus(1)
from LSM9DS0 import *
from LSM9DS1 import *
from LSM6DSL import *
from LIS3MDL import *
import time




BerryIMUversion = 99





def detectIMU():
    #Detect which version of BerryIMU is connected using the 'who am i' register
    #BerryIMUv1 uses the LSM9DS0
    #BerryIMUv2 uses the LSM9DS1
    #BerryIMUv3 uses the LSM6DSL and LIS3MDL
 
    global BerryIMUversion


    try:
        #Check for BerryIMUv1 (LSM9DS0)
        #If no LSM9DS0 is connected, there will be an I2C bus error and the program will exit.
        #This section of code stops this from happening.
        LSM9DS0_WHO_G_response = (bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_WHO_AM_I_G))
        LSM9DS0_WHO_XM_response = (bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_WHO_AM_I_XM))
    except IOError as e:
        print('')        #need to do something here, so we just print a space
    else:
        if (LSM9DS0_WHO_G_response == 0xd4) and (LSM9DS0_WHO_XM_response == 0x49):
            print("Found BerryIMUv1 (LSM9DS0)")
            BerryIMUversion = 1


    try:
        #Check for BerryIMUv2 (LSM9DS1)
        #If no LSM9DS1 is connnected, there will be an I2C bus error and the program will exit.
        #This section of code stops this from happening.
        LSM9DS1_WHO_XG_response = (bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_WHO_AM_I_XG))
        LSM9DS1_WHO_M_response = (bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_WHO_AM_I_M))

    except IOError as f:
        print('')        #need to do something here, so we just print a space
    else:
        if (LSM9DS1_WHO_XG_response == 0x68) and (LSM9DS1_WHO_M_response == 0x3d):
            print("Found BerryIMUv2 (LSM9DS1)")
            BerryIMUversion = 2

    try:
        #Check for BerryIMUv3 (LSM6DSL and LIS3MDL)
        #If no LSM6DSL or LIS3MDL is connected, there will be an I2C bus error and the program will exit.
        #This section of code stops this from happening.
        LSM6DSL_WHO_AM_I_response = (bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_WHO_AM_I))
        LIS3MDL_WHO_AM_I_response = (bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_WHO_AM_I))

    except IOError as f:
        print('')        #need to do something here, so we just print a space
    else:
        if (LSM6DSL_WHO_AM_I_response == 0x6A) and (LIS3MDL_WHO_AM_I_response == 0x3D):
            print("Found BerryIMUv3 (LSM6DSL and LIS3MDL)")
            BerryIMUversion = 3
    time.sleep(1)














def writeByte(device_address,register,value):
    bus.write_byte_data(device_address, register, value)



def readACCx():
    acc_l = 0
    acc_h = 0
    if(BerryIMUversion == 1):
        acc_l = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_X_L_A)
        acc_h = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_X_H_A)
    elif(BerryIMUversion == 2):
        acc_l = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_X_L_XL)
        acc_h = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_X_H_XL)
    elif(BerryIMUversion == 3):
        acc_l = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTX_L_XL)
        acc_h = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTX_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536



 


def readACCy():
    acc_l = 0
    acc_h = 0
    if(BerryIMUversion == 1):
        acc_l = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Y_L_A)
        acc_h = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Y_H_A)
    elif(BerryIMUversion == 2):
        acc_l = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Y_L_XL)
        acc_h = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Y_H_XL)
    elif(BerryIMUversion == 3):
        acc_l = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTY_L_XL)
        acc_h = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTY_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCz():
    acc_l = 0
    acc_h = 0
    if(BerryIMUversion == 1):
        acc_l = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Z_L_A)
        acc_h = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Z_H_A)
    elif(BerryIMUversion == 2):
        acc_l = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Z_L_XL)
        acc_h = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Z_H_XL)
    elif(BerryIMUversion == 3):
        acc_l = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTZ_L_XL)
        acc_h = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTZ_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readGYRx():
    gyr_l = 0
    gyr_h = 0
    if(BerryIMUversion == 1):
        gyr_l = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_X_L_G)
        gyr_h = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_X_H_G)
    elif(BerryIMUversion == 2):
        gyr_l = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_X_L_G)
        gyr_h = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_X_H_G)
    elif(BerryIMUversion == 3):
        gyr_l = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTX_L_G)
        gyr_h = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTX_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536


def readGYRy():
    gyr_l = 0
    gyr_h = 0
    if(BerryIMUversion == 1):
        gyr_l = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Y_L_G)
        gyr_h = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Y_H_G)
    elif(BerryIMUversion == 2):
        gyr_l = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Y_L_G)
        gyr_h = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Y_H_G)
    elif(BerryIMUversion == 3):
        gyr_l = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTY_L_G)
        gyr_h = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTY_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
    gyr_l = 0
    gyr_h = 0
    if(BerryIMUversion == 1):
        gyr_l = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Z_L_G)
        gyr_h = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Z_H_G)
    elif(BerryIMUversion == 2):
        gyr_l = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Z_L_G)
        gyr_h = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Z_H_G)
    elif(BerryIMUversion == 3):
        gyr_l = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTZ_L_G)
        gyr_h = bus.read_byte_data(LSM6DSL_ADDRESS, LSM6DSL_OUTZ_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536


def readMAGx():
    mag_l = 0
    mag_h = 0
    if(BerryIMUversion == 1):
        mag_l = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_X_L_M)
        mag_h = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_X_H_M)
    elif(BerryIMUversion == 2):
        mag_l = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_X_L_M)
        mag_h = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_X_H_M)
    elif(BerryIMUversion == 3):
        mag_l = bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_OUT_X_L)
        mag_h = bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_OUT_X_H)

    mag_combined = (mag_l | mag_h <<8)
    return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readMAGy():
    mag_l = 0
    mag_h = 0
    if(BerryIMUversion == 1):
        mag_l = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Y_L_M)
        mag_h = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Y_H_M)
    elif(BerryIMUversion == 2):
        mag_l = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Y_L_M)
        mag_h = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Y_H_M)
    elif(BerryIMUversion == 3):
        mag_l = bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_OUT_Y_L)
        mag_h = bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_OUT_Y_H)

    mag_combined = (mag_l | mag_h <<8)
    return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readMAGz():
    mag_l = 0
    mag_h = 0
    if(BerryIMUversion == 1):
        mag_l = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Z_L_M)
        mag_h = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Z_H_M)
    elif(BerryIMUversion == 2):
        mag_l = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Z_L_M)
        mag_h = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Z_H_M)
    elif(BerryIMUversion == 3):
        mag_l = bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_OUT_Z_L)
        mag_h = bus.read_byte_data(LIS3MDL_ADDRESS, LIS3MDL_OUT_Z_H)

    mag_combined = (mag_l | mag_h <<8)
    return mag_combined  if mag_combined < 32768 else mag_combined - 65536



def initIMU():

    if(BerryIMUversion == 1):   #For BerryIMUv1
        #initialise the accelerometer
        writeByte(LSM9DS0_ACC_ADDRESS,LSM9DS0_CTRL_REG1_XM, 0b01100111)  #z,y,x axis enabled, continuos update,  100Hz data rate
        writeByte(LSM9DS0_ACC_ADDRESS,LSM9DS0_CTRL_REG2_XM, 0b00011000)  #+/- 8G full scale

        #initialise the magnetometer
        writeByte(LSM9DS0_MAG_ADDRESS,LSM9DS0_CTRL_REG5_XM, 0b11110000)  #Temp enable, M data rate = 50Hz
        writeByte(LSM9DS0_MAG_ADDRESS,LSM9DS0_CTRL_REG6_XM, 0b01100000)  #+/- 12gauss
        writeByte(LSM9DS0_MAG_ADDRESS,LSM9DS0_CTRL_REG7_XM, 0b00000000)  #Continuous-conversion mode

        #initialise the gyroscope
        writeByte(LSM9DS0_GYR_ADDRESS,LSM9DS0_CTRL_REG1_G, 0b00001111)   #Normal power mode, all axes enabled
        writeByte(LSM9DS0_GYR_ADDRESS,LSM9DS0_CTRL_REG4_G, 0b00110000)   #Continuos update, 2000 dps full scale

    elif(BerryIMUversion == 2):       #For BerryIMUv2
        #initialise the accelerometer
        writeByte(LSM9DS1_ACC_ADDRESS,LSM9DS1_CTRL_REG5_XL,0b00111000)   #z, y, x axis enabled for accelerometer
        writeByte(LSM9DS1_ACC_ADDRESS,LSM9DS1_CTRL_REG6_XL,0b00111000)   #+/- 8g

        #initialise the gyroscope
        writeByte(LSM9DS1_GYR_ADDRESS,LSM9DS1_CTRL_REG4,0b00111000)      #z, y, x axis enabled for gyro
        writeByte(LSM9DS1_GYR_ADDRESS,LSM9DS1_CTRL_REG1_G,0b10111000)    #Gyro ODR = 476Hz, 2000 dps
        writeByte(LSM9DS1_GYR_ADDRESS,LSM9DS1_ORIENT_CFG_G,0b10111000)   #Swap orientation

        #initialise the magnetometer
        writeByte(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG1_M, 0b10011100)    #Temp compensation enabled,Low power mode mode,80Hz ODR
        writeByte(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG2_M, 0b01000000)    #+/- 2gauss
        writeByte(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG3_M, 0b00000000)    #continuos update
        writeByte(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG4_M, 0b00000000)    #lower power mode for Z axis

    elif(BerryIMUversion == 3):       #For BerryIMUv3
        #initialise the accelerometer
        writeByte(LSM6DSL_ADDRESS,LSM6DSL_CTRL1_XL,0b10011111)           #ODR 3.33 kHz, +/- 8g , BW = 400hz
        writeByte(LSM6DSL_ADDRESS,LSM6DSL_CTRL3_C,0b01000100)            #Enable Block Data update, increment during multi byte read

        #initialise the gyroscope
        writeByte(LSM6DSL_ADDRESS,LSM6DSL_CTRL2_G,0b10011100)            #ODR 3.3 kHz, 2000 dps

        #initialise the magnetometer
        writeByte(LIS3MDL_ADDRESS,LIS3MDL_CTRL_REG1, 0b11011100)         # Temp sesnor enabled, High performance, ODR 80 Hz, FAST ODR disabled and Selft test disabled.
        writeByte(LIS3MDL_ADDRESS,LIS3MDL_CTRL_REG2, 0b00100000)         # +/- 8 gauss
        writeByte(LIS3MDL_ADDRESS,LIS3MDL_CTRL_REG3, 0b00000000)         # Continuous-conversion mode


