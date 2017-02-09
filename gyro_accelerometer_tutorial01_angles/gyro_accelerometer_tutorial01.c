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
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "sensor.c"

#define DT 0.02 // [s/loop] loop period. 20ms
#define AA 0.97 // complementary filter constant

#define ACCELEROMETER_GAIN 0.0573 // [deg /     LSB]
#define GYROSCOPE_GAIN 0.070      // [deg / s / LSB]

#ifndef PI
#define PI 3.14159265
#endif // !PI

#define RADIAN_TO_DEGREE 180 / PI

void intHandler(int sig)
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
	signal(SIGINT, intHandler);

	enableIMU();

	float gyroscope_angle_X = 0;
	float gyroscope_angle_Y = 0;
	float gyroscope_angle_Z = 0;

	float real_angle_X = 0;
	float real_angle_Y = 0;

	char IMU_upside_down = 0;

	while( 1 ) {
		int start_time = getMillisecondsTime();
		
		// Read accelerometer and gyroscope data
		int16_t accelerometer_raw[3];
		int16_t gyroscope_raw[3];

		readAccelerometer(accelerometer_raw);
		readGyroscope    (gyroscope_raw    );
		
		// Convert Gyroscope raw values to degrees per second
		float gyroscope_rate_X = gyroscope_raw[0] * GYROSCOPE_GAIN;
		float gyroscope_rate_Y = gyroscope_raw[1] * GYROSCOPE_GAIN;
		float gyroscope_rate_Z = gyroscope_raw[2] * GYROSCOPE_GAIN;
		
		//Calculate the angles from the gyro
		gyroscope_angle_X += gyroscope_rate_X * DT;
		gyroscope_angle_Y += gyroscope_rate_Y * DT;
		gyroscope_angle_Z += gyroscope_rate_Z * DT;
		
		//Convert Accelerometer values to degrees
		float accelerometer_angle_X = (atan2f(accelerometer_raw[1], accelerometer_raw[2]) + PI) * RADIAN_TO_DEGREE;
		float accelerometer_angle_Y = (atan2f(accelerometer_raw[2], accelerometer_raw[0]) + PI) * RADIAN_TO_DEGREE;
		
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
		
		// Complementary filter is used to combine the accelerometer and gyroscope values
		real_angle_X = AA * (real_angle_X + gyroscope_rate_X * DT) + (1 - AA) * accelerometer_angle_X;
		real_angle_Y = AA * (real_angle_Y + gyroscope_rate_Y * DT) + (1 - AA) * accelerometer_angle_Y;
		
		printf( "Gyroscope [%7.3f %7.3f], Accelerometer [%7.3f %7.3f], Result [%7.3f %7.3f]\n",
				gyroscope_angle_X, gyroscope_angle_Y, accelerometer_angle_X, accelerometer_angle_Y, real_angle_X, real_angle_Y);

		//printf(" GyroX  %7.3f \t AccXangle \e[m %7.3f \t \033[22;31mCFangleX %7.3f\033[0m\t GyroY  %7.3f \t AccYangle %7.3f \t \033[22;36mCFangleY %7.3f\t\033[0m\n",gyroXangle,AccXangle,CFangleX,gyroYangle,AccYangle,CFangleY);
		
		// Each loop should be at least 20ms
		while( getMillisecondsTime() - start_time < DT * 1000 )
		    usleep(100);
		
		printf("Loop time %d ms. ", getMillisecondsTime() - start_time);
	}
}

