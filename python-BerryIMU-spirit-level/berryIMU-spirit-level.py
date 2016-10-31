#!/usr/bin/python
#
#	This program  reads the angles from the acceleromter, gyrscope
#	and converts these to a spirit level
#
#	http://ozzmaker.com/
#
#    Copyright (C) 2016  Mark Williams
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    Library General Public License for more details.
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
#    MA 02111-1307, USA

import smbus, pygame, os
import time
import math
from LSM9DS0 import *
import datetime
bus = smbus.SMBus(1)


RAD_TO_DEG = 57.29578
M_PI = 3.14159265358979323846
G_GAIN = 0.070  # [deg/s/LSB]  If you change the dps for gyro, you need to update this value accordingly
AA =  0.40      # Complementary filter constant



def writeACC(register,value):
        bus.write_byte_data(ACC_ADDRESS , register, value)
        return -1



def writeGRY(register,value):
        bus.write_byte_data(GYR_ADDRESS, register, value)
        return -1



def readACCx():
        acc_l = bus.read_byte_data(ACC_ADDRESS, OUT_X_L_A)
        acc_h = bus.read_byte_data(ACC_ADDRESS, OUT_X_H_A)
	acc_combined = (acc_l | acc_h <<8)

	return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCy():
        acc_l = bus.read_byte_data(ACC_ADDRESS, OUT_Y_L_A)
        acc_h = bus.read_byte_data(ACC_ADDRESS, OUT_Y_H_A)
	acc_combined = (acc_l | acc_h <<8)

	return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCz():
        acc_l = bus.read_byte_data(ACC_ADDRESS, OUT_Z_L_A)
        acc_h = bus.read_byte_data(ACC_ADDRESS, OUT_Z_H_A)
	acc_combined = (acc_l | acc_h <<8)

	return acc_combined  if acc_combined < 32768 else acc_combined - 65536



def readGYRx():
        gyr_l = bus.read_byte_data(GYR_ADDRESS, OUT_X_L_G)
        gyr_h = bus.read_byte_data(GYR_ADDRESS, OUT_X_H_G)
        gyr_combined = (gyr_l | gyr_h <<8)

        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536
  

def readGYRy():
        gyr_l = bus.read_byte_data(GYR_ADDRESS, OUT_Y_L_G)
        gyr_h = bus.read_byte_data(GYR_ADDRESS, OUT_Y_H_G)
        gyr_combined = (gyr_l | gyr_h <<8)

        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
        gyr_l = bus.read_byte_data(GYR_ADDRESS, OUT_Z_L_G)
        gyr_h = bus.read_byte_data(GYR_ADDRESS, OUT_Z_H_G)
        gyr_combined = (gyr_l | gyr_h <<8)

        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536



	
#initialise the accelerometer
writeACC(CTRL_REG1_XM, 0b01100111) #z,y,x axis enabled, continuos update,  100Hz data rate
writeACC(CTRL_REG2_XM, 0b00100000) #+/- 16G full scale


#initialise the gyroscope
writeGRY(CTRL_REG1_G, 0b00001111) #Normal power mode, all axes enabled
writeGRY(CTRL_REG4_G, 0b00110000) #Continuos update, 2000 dps full scale

gyroXangle = 0.0
gyroYangle = 0.0
gyroZangle = 0.0
CFangleX = 0.0
CFangleY = 0.0
CFangleZ = 0.0

a = datetime.datetime.now()







#####pygame setup., which is a wrapper for SDL#####
os.putenv('SDL_FBDEV', '/dev/fb1')				#Set the display to fb1, which is the small TFT
pygame.display.init()							#Initialise pygame.  


screen = pygame.display.set_mode ( ( 480,320 ))				#Set display mode
screen_rect = screen.get_rect()								#Get the rectangular area of the screen Surface

#Load images and convert to the correct format. Alpha is needed for PNGs to preserve alpha channel
img_background = pygame.image.load ( "spirit-level-background.jpg" ).convert()
img_Hbubble = pygame.image.load ( "spirit-level-Hbubble.png" ).convert_alpha()
img_Vbubble = pygame.image.load ( "spirit-level-Vbubble.png" ).convert_alpha()
img_Cbubble = pygame.image.load ( "spirit-level-Cbubble.png" ).convert_alpha()
img_overlay = pygame.image.load ( "spirit-level-overlay.png" ).convert_alpha()

#These images are used when only displaying the horizontal bar
img_background_H_only = pygame.image.load ( "spirit-level-H-only-background.jpg" ).convert()
img_Hbubble_H_only = pygame.image.load ( "spirit-level-H-only-bubble.png" ).convert_alpha()	
img_overlay_H_only = pygame.image.load ( "spirit-level-H-only-overlay.png" ).convert_alpha()

pygame.mouse.set_visible(False)								#Disable the mouse cursor

a = datetime.datetime.now()


#Slider limits for bubbles
hSlideLimit = [62,252]
vSlideLimit = [38,230]
ChSlideLimit = [95,267]
CvSlideLimit = [31,203]
angleLimit  = 90
#these functions are used to scale the IMU readings from the angleLimit, to be within the slider of the bubble view window.
def scaleH(value):
	min = -angleLimit
	max = angleLimit
	minScale = hSlideLimit[0]
	maxScale = hSlideLimit[1]
	scaled = minScale + (value - min)/(max-min) * (maxScale - minScale)
	return scaled

def scaleV(value):
	
	min = -angleLimit
	max = angleLimit
	minScale = vSlideLimit[0]
	maxScale = vSlideLimit[1]
	scaled = minScale + (value - min)/(max-min) * (maxScale - minScale)
	
	return scaled


#These two are used to scale the value appropriately for the bubble within the circle
def scaleCH(value):
	min = -angleLimit
	max = angleLimit
	minScale = ChSlideLimit[0]
	maxScale = ChSlideLimit[1] -36	#36 is the width of the bubble
	scaled = minScale + (value - min)/(max-min) * (maxScale - minScale)
	return scaled

def scaleCV(value):
	min = -angleLimit
	max = angleLimit
	minScale = CvSlideLimit[0]
	maxScale = CvSlideLimit[1] - 36		#36 is the width of the bubble
	scaled = minScale + (value - min)/(max-min) * (maxScale - minScale)
	return scaled

def scaleHonly(value):
	min = -angleLimit+20
	max = angleLimit-20
	minScale = 53
	maxScale = 430 - 77		#36 is the width of the bubble
	scaled = minScale + (value - min)/(max-min) * (maxScale - minScale)
	return scaled


#Used to detect if apoint(X and R) is within a circle
#Where circle_x and circle_y is the center coordinates of the circle, r is the radius of the circle.
#http://stackoverflow.com/questions/481144/equation-for-testing-if-a-point-is-inside-a-circle
def is_in_circle(circle_x, circle_y, r, x, y):
    d = math.sqrt((x - circle_x) ** 2 + (y - circle_y) ** 2)
    return d <= r


#used for low pass filter
oldXAccRawValue = 0
oldYAccRawValue = 0
oldZAccRawValue = 0

while True:
	
	
	
	#Read the accelerometer,gyroscope and magnetometer values
	ACCx = readACCx()
	ACCy = readACCy()
	ACCz = readACCz()
	GYRx = readGYRx()
	GYRy = readGYRy()
	GYRz = readGYRz()

	
	##Calculate loop Period(LP). How long between Gyro Reads
	b = datetime.datetime.now() - a
	a = datetime.datetime.now()
	LP = b.microseconds/(1000000*1.0)
	print "Loop Time | %5.2f|" % ( LP ),
	
	
	
	
	#Apply low pass filter to reduce noise
	ACC_LPF_FACTOR = 0.05
	ACCx =  ACCx  * ACC_LPF_FACTOR + oldXAccRawValue*(1 - ACC_LPF_FACTOR);
	ACCy =  ACCy  * ACC_LPF_FACTOR + oldYAccRawValue*(1 - ACC_LPF_FACTOR);
	ACCz =  ACCz  * ACC_LPF_FACTOR + oldZAccRawValue*(1 - ACC_LPF_FACTOR);
	oldXAccRawValue = ACCx;
	oldYAccRawValue = ACCy;
	oldZAccRawValue = ACCz;
	
	
	#Convert Gyro raw to degrees per second
	rate_gyr_x =  GYRx * G_GAIN
	rate_gyr_y =  GYRy * G_GAIN
	rate_gyr_z =  GYRz * G_GAIN


	#Calculate the angles from the gyro. 
	gyroXangle+=rate_gyr_x*LP
	gyroYangle+=rate_gyr_y*LP
	gyroZangle+=rate_gyr_z*LP


	##Convert Accelerometer values to degrees
	AccXangle =  (math.atan2(ACCy,ACCz)+M_PI)*RAD_TO_DEG
	AccYangle =  (math.atan2(ACCz,ACCx)+M_PI)*RAD_TO_DEG
	AccZangle =  (math.atan2(ACCy,ACCx)+M_PI)*RAD_TO_DEG
	
	####################################################################
	######################Correct rotation value########################
	####################################################################
	#Change the rotation value of the accelerometer to -/+ 180 and
	#move the Y axis '0' point to up.
	#
	#Two different pieces of code are used depending on how your IMU is mounted.
	#If IMU is up the correct way, Skull logo is facing down, Use these lines
	#AccXangle -= 180.0
	#if AccYangle > 90:
	#	AccYangle -= 270.0
	#else:
	#	AccYangle += 90.0
	#
	#
	#
	#
	#If IMU is upside down E.g Skull logo is facing up;
	if AccXangle >180:
	        AccXangle -= 360.0
	AccYangle-=90
	if (AccYangle >180):
	        AccYangle -= 360.0
	############################ END ##################################
	if AccZangle >180:
	        AccZangle -= 360.0

	        
	#Complementary filter used to combine the accelerometer and gyro values.
	CFangleX=AA*(CFangleX+rate_gyr_x*LP) +(1 - AA) * AccXangle
	CFangleY=AA*(CFangleY+rate_gyr_y*LP) +(1 - AA) * AccYangle
	CFangleZ=AA*(CFangleZ+rate_gyr_z*LP) +(1 - AA) * AccZangle


	if 0:			#Change to '0' to stop showing the angles from the accelerometer
 		print ("\033[1;34;40mACCX Angle %5.2f ACCY Angle %5.2f  ACCZ Angle %5.2f  \033[0m  " % (AccXangle, AccYangle, AccZangle)),
	
	if 0:			#Change to '0' to stop  showing the angles from the gyro
		print ("\033[1;31;40m\tGRYX Angle %5.2f  GYRY Angle %5.2f  GYRZ Angle %5.2f" % (gyroXangle,gyroYangle,gyroZangle)),

 	if 1:			#Change to '0' to stop  showing the angles from the complementary filter
		print ("\033[1;35;40m   \tCFangleX Angle %5.2f \033[1;36;40m  CFangleY Angle %5.2f  \033[1;33;40m  CFangleZ Angle %5.2f\33[1;32;40m" % (CFangleX,CFangleY,CFangleZ))



	
	
	#limit the angles
	if CFangleX > angleLimit:
		CFangleX = angleLimit
	if CFangleX < -angleLimit:
		CFangleX = -angleLimit
	if CFangleY > angleLimit:
		CFangleY = angleLimit
	if CFangleY < -angleLimit:
		CFangleY = -angleLimit
		
	
	if CFangleY > 70 or CFangleY <-70:					#If Y angle is not between -70 and 70, then only show large horizontal bar.
		ACC_LPF_FACTOR = 0.05						#lower low pass filter
		hBposition = (int(scaleHonly(CFangleZ	)),190)
		screen.blit(img_background_H_only, screen_rect)			#Blit the new image to the surface
		screen.blit(img_Hbubble_H_only, hBposition)			#Blit the new image to the surface
		screen.blit(img_overlay_H_only, screen_rect)			#Blit the new image to the surface
	else:
		ACC_LPF_FACTOR = 0.3
		
		hBposition = (int(scaleH(CFangleX)),244)
		vBposition = (398,int(scaleV(CFangleY)))
		newcBposition = (int(scaleCH(CFangleX)),int(scaleCV(CFangleY)))
		
		#confirm that center bubble is within the circle;
		if(is_in_circle(181,117,90,newcBposition[0],newcBposition[1])):
			cBposition = newcBposition
		#Blit the new images to the surface
		screen.blit(img_background, screen_rect)
		screen.blit(img_Hbubble, hBposition)
		screen.blit(img_Vbubble, vBposition)
		screen.blit(img_Cbubble, cBposition)
		screen.blit(img_overlay, screen_rect)
	
	pygame.display.flip()									#Show the new image on the TFT
	
