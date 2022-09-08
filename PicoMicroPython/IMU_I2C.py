from LSM6DSL import *
import machine

# Placed methods inside Class to eliminate need to set pins in library.
# Commented Code below to be placed in your Raspberry Pi Pico Project

# import IMU_I2C
# IMU_I2C_NUMBER = const(1) # 0 or 1
# IMU_SDA_PIN    = const(6) # Pins for I2C0 [0, 4, 8, 12, 16, 20] for I2C1 [2, 6, 10, 14, 18, 26]
# IMU_SCL_PIN    = const(7) # Pins for I2C0 [1, 5, 9, 13, 17, 21] for I2C1 [3, 7, 11, 15, 19, 27]
# IMU = IMU_I2C.IMUClass(IMU_I2C_NUMBER, IMU_SDA_PIN, IMU_SCL_PIN) #Initialize the accelerometer, gyroscope


class IMUClass:
    def __init__(self, i2c_number, i2c_sda, i2c_scl, i2c_freq=100000):
        self.i2c = machine.I2C(i2c_number, sda=machine.Pin(i2c_sda),
                               scl=machine.Pin(i2c_scl), freq=i2c_freq)
        self.initIMU()

    def writeReg(self, reg_address, data):
            res = bytearray(2)    
            self.i2c.writeto_mem(LSM6DSL_ADDRESS, reg_address, bytes([data]))

    def readReg(self, reg_address):
            res = self.i2c.readfrom_mem(LSM6DSL_ADDRESS, reg_address, 1)
            return res

    def readACCx(self):
        acc_l = self.readReg( LSM6DSL_OUTX_L_XL)
        acc_h = self.readReg( LSM6DSL_OUTX_H_XL)
        acc_combined = (ord(acc_l) | ord(acc_h) <<8) #ord is to get int representation from a byte
        return acc_combined  if acc_combined < 32768 else acc_combined - 65536

    def readACCy(self):
        acc_l = self.readReg( LSM6DSL_OUTY_L_XL)
        acc_h = self.readReg( LSM6DSL_OUTY_H_XL)
        acc_combined = (ord(acc_l) | ord(acc_h) <<8) #ord is to get int representation from a byte
        return acc_combined  if acc_combined < 32768 else acc_combined - 65536

    def readACCz(self):
        acc_l = self.readReg( LSM6DSL_OUTZ_L_XL)
        acc_h = self.readReg( LSM6DSL_OUTZ_H_XL)
        acc_combined = (ord(acc_l) | ord(acc_h) <<8) #ord is to get int representation from a byte
        return acc_combined  if acc_combined < 32768 else acc_combined - 65536

    def readGYRx(self):
        gyr_l = self.readReg(LSM6DSL_OUTX_L_G)
        gyr_h = self.readReg(LSM6DSL_OUTX_H_G)
        gyr_combined = (ord(gyr_l) | ord(gyr_h) <<8) #ord is to get int representation from a byte
        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

    def readGYRy(self):
        gyr_l = self.readReg(LSM6DSL_OUTY_L_G)
        gyr_h = self.readReg(LSM6DSL_OUTY_H_G)
        gyr_combined = (ord(gyr_l) | ord(gyr_h) <<8) #ord is to get int representation from a byte
        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

    def readGYRz(self):
        gyr_l = self.readReg(LSM6DSL_OUTZ_L_G)
        gyr_h = self.readReg(LSM6DSL_OUTZ_H_G)
        gyr_combined = (ord(gyr_l) | ord(gyr_h) <<8) #ord is to get int representation from a byte
        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

    def initIMU(self):
        print(self.i2c.scan())
        #initialise the accelerometer
        self.writeReg(LSM6DSL_CTRL1_XL,0b10011111)           #ODR 3.33 kHz, +/- 8g , BW = 400hz
        self.writeReg(LSM6DSL_CTRL8_XL,0b11001000)           #Low pass filter enabled, BW9, composite filter
        self.writeReg(LSM6DSL_CTRL3_C,0b01000100)            #Enable Block Data update, increment during multi byte read

        #initialise the gyroscope
        self.writeReg(LSM6DSL_CTRL2_G,0b10011100)#ODR 3.3 kHz, 2000 dps
