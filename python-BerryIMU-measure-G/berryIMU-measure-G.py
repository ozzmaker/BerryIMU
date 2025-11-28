#!/usr/bin/python
#
#       This program demonstrates how to convert the raw values from an accelerometer to Gs
#
#       The BerryIMUv1, BerryIMUv2, BerryIMUv3 and BerryIMU320G are supported
#
#       This script is python 2.7 and 3 compatible
#
#       Feel free to do whatever you like with this code.
#       Distributed as-is; no warranty is given.
#
#       https://ozzmaker.com/accelerometer-to-g/


import time
import IMU
import sys



IMU.detectIMU()     #Detect if BerryIMU is connected.
if(IMU.BerryIMUversion == 99):
    print(" No BerryIMU found... exiting ")
    sys.exit()
IMU.initIMU()       #Initialise the accelerometer, gyroscope and compass


while True:
    #If BerryIMG320G, read High G accelerometer
    if IMU.BerryIMUversion == 320:
        ACCx_HG = IMU.readACCx_HG()
        ACCy_HG = IMU.readACCy_HG()
        ACCz_HG = IMU.readACCz_HG()
        xG_HG = (ACCx_HG * 0.976)/1000
        yG_HG = (ACCy_HG * 0.976)/1000
        zG_HG = (ACCz_HG * 0.976)/1000
        print("|| HIGH_G X = %5.3fG\tY =   %5.3fG\tZ =  %5.3fG\t" % ( yG_HG, xG_HG, zG_HG)),

    #Read the accelerometer values
    ACCx = IMU.readACCx()
    ACCy = IMU.readACCy()
    ACCz = IMU.readACCz()
    xG = (ACCx * 0.244)/1000
    yG = (ACCy * 0.244)/1000
    zG = (ACCz * 0.244)/1000

    print("||\tX = %5.3fG\tY =   %5.3fG\tZ =  %5.3fG  ||" % ( yG, xG, zG))

    #slow program down a bit, makes the output more readable
    time.sleep(0.03)
