/*
	This program  reads the angles from the accelerometer and gyroscope
	on a BerryIMU connected to a Raspberry Pi.
	http://ozzmaker.com/


    Copyright (C) 2014  Mark Williams

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
    MA 02111-1307, USA
*/

#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "sensor.c"

#define DT 0.02       // [s/loop] loop period. 20ms
#define AA 0.97       // complementary filter constant

#define A_GAIN 0.0573 // [deg/LSB]
#define G_GAIN 0.070  // [deg/s/LSB]

#ifndef PI

#define PI 3.14159265

#endif // !PI

#define RAD_TO_DEG 180 / PI

void INThandler(int sig)
{
	signal(sig, SIG_IGN);
	exit(0);
}

int getMillisecondsTime()
{
	struct timeval value;
	gettimeofday(&value, NULL);

	return (value.tv_sec) * 1000 + (value.tv_usec) / 1000;
}

int main()
{
	signal(SIGINT, INThandler);

	enableIMU();

	float gyroscore_angle_X = 0;
	float gyroscore_angle_Y = 0;
	float gyroscore_angle_Z = 0;

	float real_angle_X = 0;
	float real_angle_Y = 0;

	char IMU_upside_down = 0;

	while( 1 ) {
		int startInt = getMillisecondsTime();
		
		// read accelerometer and gyroscope data

		int16_t accRaw[3];
		int16_t gyrRaw[3];

		readAccelerometer(accRaw);
		readGyroscope(gyrRaw);
		
		//Convert Gyro raw to degrees per second
		float gyroscore_rate_X = gyrRaw[0] * G_GAIN;
		float gyroscore_rate_Y = gyrRaw[1] * G_GAIN;
		float gyroscore_rate_Z = gyrRaw[2] * G_GAIN;
		
		//Calculate the angles from the gyro
		gyroscore_angle_X += gyroscore_rate_X * DT;
		gyroscore_angle_Y += gyroscore_rate_Y * DT;
		gyroscore_angle_Z += gyroscore_rate_Z * DT;
		
		//Convert Accelerometer values to degrees
		float accelerometer_angle_X = (atan2f(accRaw[1], accRaw[2]) + PI) * RAD_TO_DEG;
		float accelerometer_angle_Y = (atan2f(accRaw[2], accRaw[0]) + PI) * RAD_TO_DEG;
		
		// Change the rotation value of the accelerometer to -/+ 180 and move the Y axis '0' point to up
		// Two different pieces of code are used depending on how your IMU is mounted
		if( IMU_upside_down ) { // if IMU is upside down
			if( accelerometer_angle_X > 180 )
				accelerometer_angle_X -= 360;
		
		    accelerometer_angle_Y -= 90;
		    
			if( accelerometer_angle_Y > 180 )
				accelerometer_angle_Y -= 360;
		}
		else { // if IMU is up the correct way
			accelerometer_angle_X -= 180;

			if (accelerometer_angle_Y > 90)
				accelerometer_angle_Y -= 270;
			else
				accelerometer_angle_Y += 90;
		}
		
		//Complementary filter used to combine the accelerometer and gyro values.
		real_angle_X = AA * (real_angle_X + gyroscore_rate_X * DT) + (1 - AA) * accelerometer_angle_X;
		real_angle_Y = AA * (real_angle_Y + gyroscore_rate_Y * DT) + (1 - AA) * accelerometer_angle_Y;
		
		printf( "Gyroscore [%7.3f %7.3f], Accelerometer [%7.3f %7.3f], Result [%7.3f %7.3f]\n",
				gyroscore_angle_X, gyroscore_angle_Y, accelerometer_angle_X, accelerometer_angle_Y, real_angle_X, real_angle_Y);

		//printf(" GyroX  %7.3f \t AccXangle \e[m %7.3f \t \033[22;31mCFangleX %7.3f\033[0m\t GyroY  %7.3f \t AccYangle %7.3f \t \033[22;36mCFangleY %7.3f\t\033[0m\n",gyroXangle,AccXangle,CFangleX,gyroYangle,AccYangle,CFangleY);
		
		//Each loop should be at least 20ms.
		while(getMillisecondsTime() - startInt < (DT*1000)) {
		    usleep(100);
		}
		
		printf("Loop time %d ms. ", getMillisecondsTime() - startInt);
	}
}

