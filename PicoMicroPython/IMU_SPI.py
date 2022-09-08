from LSM6DSL import *
import machine
import time

READ_FLAG = 0x80

##################### NOT TESTED JH ###########################################
# Placed methods inside Class to eliminate need to set pins in library.
# Code below to be placed in your Raspberry Pi Pico Project

# import IMU_SPI
# SPI_NUMBER     = const(0)      # 0, 1
# SPI_RX_PIN     = const(4)      # Pins for SPI0 [0, 4, 16] for SPI1 [8, 12]
# IMU_SPI_CS_PIN = const(5)      # Pins for SPI0 [1, 5, 17] for SPI1 [9, 13]
# SPI_SCK_PIN    = const(2)      # Pins for SPI0 [2, 6, 18] for SPI1 [10, 14]
# SPI_TX_PIN     = const(3)      # Pins for SPI0 [3, 7, 19] for SPI1 [11, 15]
# SPI_BAUD       = const(100000) # 100_000 stock
# IMU = IMU_SPI.IMUClass(SPI_NUMBER, SPI_SCK_PIN, SPI_TX_PIN, SPI_RX_PIN, IMU_SPI_CS_PIN, SPI_BAUD)#Initialize the accelerometer, gyroscope


class IMUClass:
    def __init__(self, spi_number, spi_sck, spi_tx, spi_rx, spi_cs, spi_baud=100000):
        self.spi=machine.SPI(spi_number,baudrate=spi_baud,sck=machine.Pin(spi_sck),
                             mosi=machine.Pin(spi_tx), miso=machine.Pin(spi_rx))
        self.spi_cs = machine.Pin(spi_cs, machine.Pin.OUT)
        self.initIMU()


    def readReg(self, reg_address):
        cmd = bytearray((reg_address | READ_FLAG, 0x00))
        res = bytearray(2)
        self.spi_cs.low()
        self.spi.write_readinto(cmd,res)
        self.spi_cs.high()
        return res[1]


    def writeReg(self, reg_address, data):
        cmd = bytearray((reg_address, data))
        res = bytearray(2)    
        self.spi_cs.low()
        self.spi.write_readinto(cmd,res)
        self.spi_cs.high()
        return res


    def readACCx(self):
        acc_l = 0
        acc_h = 0
        acc_l = self.readReg( LSM6DSL_OUTX_L_XL)
        acc_h = self.readReg( LSM6DSL_OUTX_H_XL)

        acc_combined = (acc_l | acc_h <<8)
        return acc_combined  if acc_combined < 32768 else acc_combined - 65536


    def readACCy(self):
        acc_l = 0
        acc_h = 0

        acc_l = self.readReg(LSM6DSL_OUTY_L_XL)
        acc_h = self.readReg(LSM6DSL_OUTY_H_XL)

        acc_combined = (acc_l | acc_h <<8)
        return acc_combined  if acc_combined < 32768 else acc_combined - 65536


    def readACCz(self):
        acc_l = 0
        acc_h = 0
     
        acc_l = self.readReg(LSM6DSL_OUTZ_L_XL)
        acc_h = self.readReg(LSM6DSL_OUTZ_H_XL)

        acc_combined = (acc_l | acc_h <<8)
        return acc_combined  if acc_combined < 32768 else acc_combined - 65536


    def readGYRx(self):
        gyr_l = 0
        gyr_h = 0

        gyr_l = self.readReg(LSM6DSL_OUTX_L_G)
        gyr_h = self.readReg(LSM6DSL_OUTX_H_G)

        gyr_combined = (gyr_l | gyr_h <<8)
        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536


    def readGYRy(self):
        gyr_l = 0
        gyr_h = 0

        gyr_l = self.readReg(LSM6DSL_OUTY_L_G)
        gyr_h = self.readReg(LSM6DSL_OUTY_H_G)

        gyr_combined = (gyr_l | gyr_h <<8)
        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536


    def readGYRz(self):
        gyr_l = 0
        gyr_h = 0

        gyr_l = self.readReg(LSM6DSL_OUTZ_L_G)
        gyr_h = self.readReg(LSM6DSL_OUTZ_H_G)

        gyr_combined = (gyr_l | gyr_h <<8)
        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

        
    def initIMU(self):

        #initialise the accelerometer
        self.writeReg(LSM6DSL_CTRL1_XL,0b10011111)           #ODR 3.33 kHz, +/- 8g , BW = 400hz
        self.writeReg(LSM6DSL_CTRL8_XL,0b11001000)           #Low pass filter enabled, BW9, composite filter
        self.writeReg(LSM6DSL_CTRL3_C,0b01000100)            #Enable Block Data update, increment during multi byte read

        #initialise the gyroscope
        self.writeReg(LSM6DSL_CTRL2_G,0b10011100)            #ODR 3.3 kHz, 2000 dps
        
