#!/usr/bin/python
#
#       This program demonstrates how to convert the raw values from an accelerometer to Gs
#
#
#       Both the BerryIMUv1 and BerryIMUv2 are supported
#
#       This script is python 2.7 and 3 compatible
#
#       Feel free to do whatever you like with this code.
#       Distributed as-is; no warranty is given.
#
#       http://ozzmaker.com/



import time
import IMU




IMU.detectIMU()     #Detect if BerryIMUv1 or BerryIMUv2 is connected.
IMU.initIMU()       #Initialise the accelerometer, gyroscope and compass




while True:


    #Read the accelerometer,gyroscope and magnetometer values
    ACCx = IMU.readACCx()
    ACCy = IMU.readACCy()
    ACCz = IMU.readACCz()
    yG = (ACCy * 0.244)/1000
    xG = (ACCy * 0.244)/1000
    zG = (ACCy * 0.244)/1000
    print("##### X = %fG  ##### Y =   %fG  ##### Z =  %fG  #####" % ( yG, xG, zG))



    #slow program down a bit, makes the output more readable
    time.sleep(0.03)
