from LSM6DSL import *
import machine
import time

READ_FLAG = 0x80



spi_sck=machine.Pin(2)
spi_tx=machine.Pin(3)
spi_rx=machine.Pin(4)
spi_cs=machine.Pin(5, machine.Pin.OUT)

spi=machine.SPI(0,baudrate=100000,sck=spi_sck, mosi=spi_tx, miso=spi_rx)


def readReg(reg_address):
        cmd = bytearray((reg_address | READ_FLAG, 0x00))
        res = bytearray(2)
        spi_cs.low()
        spi.write_readinto(cmd,res)
        spi_cs.high()
        return res[1]

def writeReg(reg_address, data):
        cmd = bytearray((reg_address, data))
        res = bytearray(2)    
        spi_cs.low()
        spi.write_readinto(cmd,res)
        spi_cs.high()
        return res
          



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

    acc_l = readReg(LSM6DSL_OUTY_L_XL)
    acc_h = readReg(LSM6DSL_OUTY_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCz():
    acc_l = 0
    acc_h = 0
 
    acc_l = readReg(LSM6DSL_OUTZ_L_XL)
    acc_h = readReg(LSM6DSL_OUTZ_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readGYRx():
    gyr_l = 0
    gyr_h = 0

    gyr_l = readReg(LSM6DSL_OUTX_L_G)
    gyr_h = readReg(LSM6DSL_OUTX_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536


def readGYRy():
    gyr_l = 0
    gyr_h = 0

    gyr_l = readReg(LSM6DSL_OUTY_L_G)
    gyr_h = readReg(LSM6DSL_OUTY_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
    gyr_l = 0
    gyr_h = 0

    gyr_l = readReg(LSM6DSL_OUTZ_L_G)
    gyr_h = readReg(LSM6DSL_OUTZ_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

    
def initIMU():

    #initialise the accelerometer
    writeReg(LSM6DSL_CTRL1_XL,0b10011111)           #ODR 3.33 kHz, +/- 8g , BW = 400hz
    writeReg(LSM6DSL_CTRL8_XL,0b11001000)           #Low pass filter enabled, BW9, composite filter
    writeReg(LSM6DSL_CTRL3_C,0b01000100)            #Enable Block Data update, increment during multi byte read

    #initialise the gyroscope
    writeReg(LSM6DSL_CTRL2_G,0b10011100)            #ODR 3.3 kHz, 2000 dps
    
    
   

