#!/usr/bin/python
#
#    This program demonstrates how to convert the raw values from an accelerometer to Gs
#
#
#    Both the BerryIMUv1 and BerryIMUv2 are supported
#
#    Feel free to do whatever you like with this code
#    Distributed as-is; no warranty is given.
#
#    http://ozzmaker.com/



import time
import IMU




IMU.detectIMU()     #Detect if BerryIMUv1 or BerryIMUv2 is connected.
IMU.initIMU()       #Initialise the accelerometer, gyroscope and compass

	


while True:
	
	
	#Read the accelerometer,gyroscope and magnetometer values
	ACCx = IMU.readACCx()
	ACCy = IMU.readACCy()
	ACCz = IMU.readACCz()

	print("##### X = %f G  #####" % ((ACCx * 0.244)/1000)),
	print(" Y =   %fG  #####" % ((ACCy * 0.244)/1000)),
	print(" Z =  %fG  #####" % ((ACCz * 0.244)/1000))

	
	
	#slow program down a bit, makes the output more readable
	time.sleep(0.03)


