#!/usr/bin/python
#
#       This is the base code needed to get usable angles from a BerryIMU
#       using connected to a Raspberry Pi Pico, using I2C or SPI
#
#       BerryIMUv3 is supported by this code
#
#
#       Feel free to do whatever you like with this code.
#       Distributed as-is; no warranty is given.
#
#       https://ozzmaker.com/berryimu/


import utime
import math
from LSM6DSL import *
import machine

#Comment out one of the below groupings

# import IMU_SPI
# SPI_NUMBER     = const(0)      # 0, 1
# SPI_RX_PIN     = const(4)      # Pins for SPI0 [0, 4, 16] for SPI1 [8, 12]
# IMU_SPI_CS_PIN = const(5)      # Pins for SPI0 [1, 5, 17] for SPI1 [9, 13]
# SPI_SCK_PIN    = const(2)      # Pins for SPI0 [2, 6, 18] for SPI1 [10, 14]
# SPI_TX_PIN     = const(3)      # Pins for SPI0 [3, 7, 19] for SPI1 [11, 15]
# SPI_BAUD       = const(100000) # 100_000 stock
# IMU = IMU_SPI.IMUClass(SPI_NUMBER, SPI_SCK_PIN, SPI_TX_PIN, SPI_RX_PIN, IMU_SPI_CS_PIN, SPI_BAUD)#Initialize the accelerometer, gyroscope


import IMU_I2C
IMU_I2C_NUMBER = const(1) # 0 or 1
IMU_SDA_PIN    = const(6) # Pins for I2C0 [0, 4, 8, 12, 16, 20] for I2C1 [2, 6, 10, 14, 18, 26]
IMU_SCL_PIN    = const(7) # Pins for I2C0 [1, 5, 9, 13, 17, 21] for I2C1 [3, 7, 11, 15, 19, 27]
IMU = IMU_I2C.IMUClass(IMU_I2C_NUMBER, IMU_SDA_PIN, IMU_SCL_PIN) #Initialize the accelerometer, gyroscope

###########


RAD_TO_DEG = const(57.29578)
M_PI = const(3.14159265358979323846)
G_GAIN = const(0.070)  # [deg/s/LSB]  If you change the dps for gyro, you need to update this value accordingly
AA =  const(0.40)      # Complementary filter constant

gyroXangle = 0.0
gyroYangle = 0.0
gyroZangle = 0.0
CFangleX = 0.0
CFangleY = 0.0


a = utime.ticks_us()

while True:
    
        
    #Read the accelerometer,gyroscope and magnetometer values
    ACCx = IMU.readACCx()
    ACCy = IMU.readACCy()
    ACCz = IMU.readACCz()
    GYRx = IMU.readGYRx()
    GYRy = IMU.readGYRy()
    GYRz = IMU.readGYRz()

    b = utime.ticks_us() - a
    a = utime.ticks_us()
    LP = b/(1000000*1.0)
    outputString = f"Loop Time {LP:5.4f}"


    #Convert Gyro raw to degrees per second
    rate_gyr_x =  GYRx * G_GAIN
    rate_gyr_y =  GYRy * G_GAIN
    rate_gyr_z =  GYRz * G_GAIN


    #Calculate the angles from the gyro.
    gyroXangle+=rate_gyr_x*LP
    gyroYangle+=rate_gyr_y*LP
    gyroZangle+=rate_gyr_z*LP


    #Convert Accelerometer values to degrees
    AccXangle =  (math.atan2(ACCy,ACCz)*RAD_TO_DEG)
    AccYangle =  (math.atan2(ACCz,ACCx)+M_PI)*RAD_TO_DEG

    #convert the values to -180 and +180
    if AccYangle > 90:
        AccYangle -= 270.0
    else:
        AccYangle += 90.0


    #Complementary filter used to combine the accelerometer and gyro values.
    CFangleX=AA*(CFangleX+rate_gyr_x*LP) +(1 - AA) * AccXangle
    CFangleY=AA*(CFangleY+rate_gyr_y*LP) +(1 - AA) * AccYangle


    xG = (ACCx * 0.244)/1000
    yG = (ACCy * 0.244)/1000
    zG = (ACCz * 0.244)/1000
    print(f"##### X = {xG:1.1f}G  ##### Y = {yG:1.1f}G  ##### Z = {zG:1.1f}G  #####")

    
    if 0:                       #Change to '0' to stop showing the angles from the accelerometer
        outputString = f"{outputString}  ACCX Angle {AccXangle:5.2f} ACCY Angle {AccYangle:5.2f}  "

    if 0:                       #Change to '0' to stop  showing the angles from the gyro
        outputString = f"{outputString}  GRYX Angle {gyroXangle:5.2f}  GYRY Angle {gyroYangle:5.2f}  GYRZ Angle {gyroZangle:5.2f}  "

    if 0:                       #Change to '0' to stop  showing the angles from the complementary filter
        outputString = f"{outputString}  CFangleX Angle {CFangleX:5.2f}   CFangleY Angle {CFangleY:5.2f}  "
#         outputString +="\t#  CFangleX Angle %5.2f   CFangleY Angle %5.2f  #" % (CFangleX,CFangleY)

    print(outputString)
    utime.sleep(0.1)
