from LSM6DSL import *
import machine

sda=machine.Pin(0)
scl=machine.Pin(1)
i2c=machine.I2C(0,sda=sda, scl=scl, freq=100000)



def writeReg(reg_address, data):
        res = bytearray(2)    
        i2c.writeto_mem(LSM6DSL_ADDRESS, reg_address, bytes([data]))

def readReg(reg_address):
        res = i2c.readfrom_mem(LSM6DSL_ADDRESS, reg_address, 1)
        return res

def readACCx():
    acc_l = readReg( LSM6DSL_OUTX_L_XL)
    acc_h = readReg( LSM6DSL_OUTX_H_XL)
    acc_combined = (ord(acc_l) | ord(acc_h) <<8) #ord is to get int representation from a byte
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536

def readACCy():
    acc_l = readReg( LSM6DSL_OUTY_L_XL)
    acc_h = readReg( LSM6DSL_OUTY_H_XL)
    acc_combined = (ord(acc_l) | ord(acc_h) <<8) #ord is to get int representation from a byte
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536

def readACCz():
    acc_l = readReg( LSM6DSL_OUTZ_L_XL)
    acc_h = readReg( LSM6DSL_OUTZ_H_XL)
    acc_combined = (ord(acc_l) | ord(acc_h) <<8) #ord is to get int representation from a byte
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536

def readGYRx():
    gyr_l = readReg(LSM6DSL_OUTX_L_G)
    gyr_h = readReg(LSM6DSL_OUTX_H_G)
    gyr_combined = (ord(gyr_l) | ord(gyr_h) <<8) #ord is to get int representation from a byte
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRy():
    gyr_l = readReg(LSM6DSL_OUTY_L_G)
    gyr_h = readReg(LSM6DSL_OUTY_H_G)
    gyr_combined = (ord(gyr_l) | ord(gyr_h) <<8) #ord is to get int representation from a byte
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
    gyr_l = readReg(LSM6DSL_OUTZ_L_G)
    gyr_h = readReg(LSM6DSL_OUTZ_H_G)
    gyr_combined = (ord(gyr_l) | ord(gyr_h) <<8) #ord is to get int representation from a byte
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def initIMU():
    print(i2c.scan())
    #initialise the accelerometer
    writeReg(LSM6DSL_CTRL1_XL,0b10011111)           #ODR 3.33 kHz, +/- 8g , BW = 400hz
    writeReg(LSM6DSL_CTRL8_XL,0b11001000)           #Low pass filter enabled, BW9, composite filter
    writeReg(LSM6DSL_CTRL3_C,0b01000100)            #Enable Block Data update, increment during multi byte read

    #initialise the gyroscope
    writeReg(LSM6DSL_CTRL2_G,0b10011100)#ODR 3.3 kHz, 2000 dps


