#!/usr/bin/python
#   This script is used to calibrate the compass on a BerryIMU.
#
#   Start this program and rotate your BerryIMU in all directions.
#   You will see the maximum and minimum values change.
#   After about 30secs or when the values are not changing, press Ctrl-C.
#   The script will printout some text which you then need to add to
#   berryIMU.py or berryIMU-simple.py
#
#   The BerryIMUv1, BerryIMUv2 and BerryIMUv3 are supported
#
#   This script is python 2.7 and 3 compatible
#
#   Feel free to do whatever you like with this code.
#   Distributed as-is; no warranty is given.
#
#   http://ozzmaker.com/


import sys,signal,os
import time
import math

import IMU
import datetime


def handle_ctrl_c(signal, frame):
    print(" ")
    print("magXmin = %i"%  (magXmin))
    print("magYmin = %i"%  (magYmin))
    print("magZmin = %i"%  (magZmin))
    print("magXmax = %i"%  (magXmax))
    print("magYmax = %i"%  (magYmax))
    print("magZmax = %i"%  (magZmax))
    sys.exit(130) # 130 is standard exit code for ctrl-c



IMU.detectIMU()
IMU.initIMU()

#This will capture exit when using Ctrl-C
signal.signal(signal.SIGINT, handle_ctrl_c)


a = datetime.datetime.now()


#Preload the variables used to keep track of the minimum and maximum values
magXmin = 32767
magYmin = 32767
magZmin = 32767
magXmax = -32767
magYmax = -32767
magZmax = -32767



while True:

    #Read magnetometer values
    MAGx = IMU.readMAGx()
    MAGy = IMU.readMAGy()
    MAGz = IMU.readMAGz()

    if MAGx > magXmax:
        magXmax = MAGx
    if MAGy > magYmax:
        magYmax = MAGy
    if MAGz > magZmax:
        magZmax = MAGz

    if MAGx < magXmin:
        magXmin = MAGx
    if MAGy < magYmin:
        magYmin = MAGy
    if MAGz < magZmin:
        magZmin = MAGz

    print((" magXmin  %i  magYmin  %i  magZmin  %i  ## magXmax  %i  magYmax  %i  magZmax %i  " %(magXmin,magYmin,magZmin,magXmax,magYmax,magZmax)))



    #slow program down a bit, makes the output more readable
    time.sleep(0.03)


