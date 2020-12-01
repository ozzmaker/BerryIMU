from LSM6DSL import *
import spidev
import time

READ_FLAG = 0x80
__MULTIPLE_READ = 0x40


BerryIMUversion = 99


spi = spidev.SpiDev()


def bus_open( ):
        spi.open(0,0 )
        spi.max_speed_hz = 10000000

def readReg( reg_address):
        #bus_open()
        tx = [reg_address | READ_FLAG, 0x00]
        rx = spi.xfer2(tx)
        return rx[1]

def writeReg(reg_address, data):
        #bus_open()
        tx = [reg_address, data]
        rx = spi.xfer2(tx)
        return rx
        



def detectIMU():
    #The accelerometer and gyrscope on the BerryIMUv3 is a LSM6DSL, here we will try and see if it is connected.

    global BerryIMUversion

    bus_open()
    try:
        #Check for LSM6DSL on the BerryIMUv3
        #If no LSM6DSL, there will be an I2C bus error and the program will exit.
        #This section of code stops this from happening.
        LSM6DSL_WHO_AM_I_response = readReg(LSM6DSL_WHO_AM_I)


    except IOError as f:
        print('')        #need to do something here, so we just print a space
    else:
        if (LSM6DSL_WHO_AM_I_response == 0x6A) :
            print("Found BerryIMUv3 (LSM6DSL)")
            BerryIMUversion = 3
    time.sleep(1)



def writeByte(device_address,register,value):
    bus.write_byte_data(device_address, register, value)



def readACCx():
    acc_l = 0
    acc_h = 0
    acc_l = readReg( LSM6DSL_OUTX_L_XL)
    acc_h = readReg( LSM6DSL_OUTX_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536



 


def readACCy():
    acc_l = 0
    acc_h = 0

    acc_l = readReg( LSM6DSL_OUTY_L_XL)
    acc_h = readReg( LSM6DSL_OUTY_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCz():
    acc_l = 0
    acc_h = 0
 
    acc_l = readReg( LSM6DSL_OUTZ_L_XL)
    acc_h = readReg( LSM6DSL_OUTZ_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readGYRx():
    gyr_l = 0
    gyr_h = 0

    gyr_l = readReg( LSM6DSL_OUTX_L_G)
    gyr_h = readReg( LSM6DSL_OUTX_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536


def readGYRy():
    gyr_l = 0
    gyr_h = 0

    gyr_l = readReg( LSM6DSL_OUTY_L_G)
    gyr_h = readReg( LSM6DSL_OUTY_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
    gyr_l = 0
    gyr_h = 0

    gyr_l = readReg( LSM6DSL_OUTZ_L_G)
    gyr_h = readReg( LSM6DSL_OUTZ_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536



def initIMU():
    #initialise the accelerometer
    writeReg(LSM6DSL_CTRL1_XL,0b10011111)           #ODR 3.33 kHz, +/- 8g , BW = 400hz
    writeReg(LSM6DSL_CTRL8_XL,0b11001000)           #Low pass filter enabled, BW9, composite filter
    writeReg(LSM6DSL_CTRL3_C,0b01000100)            #Enable Block Data update, increment during multi byte read

    #initialise the gyroscope
    writeReg(LSM6DSL_CTRL2_G,0b10011100)            #ODR 3.3 kHz, 2000 dps



