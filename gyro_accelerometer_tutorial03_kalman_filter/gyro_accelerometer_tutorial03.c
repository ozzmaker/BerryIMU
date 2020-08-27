
/*
    This program  reads the angles from the accelerometer and gyroscope
    on a BerryIMU connected to a Raspberry Pi.

    A Kalman filter is used.

    The BerryIMUv1, BerryIMUv2 and BerryIMUv3 are supported.

    Feel free to do whatever you like with this code
    Distributed as-is; no warranty is given.

    https://ozzmaker.com/berryimu/
*/

#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "IMU.c"


#define DT 0.02         // [s/loop] loop period. 20ms
#define AA 0.97         // complementary filter constant

#define A_GAIN 0.0573      // [deg/LSB]
#define G_GAIN 0.070     // [deg/s/LSB]
#define RAD_TO_DEG 57.29578
#define M_PI 3.14159265358979323846


//Used by Kalman Filters
float Q_angle  =  0.01;
float Q_gyro   =  0.0003;
float R_angle  =  0.01;
float x_bias = 0;
float y_bias = 0;
float XP_00 = 0, XP_01 = 0, XP_10 = 0, XP_11 = 0;
float YP_00 = 0, YP_01 = 0, YP_10 = 0, YP_11 = 0;
float KFangleX = 0.0;
float KFangleY = 0.0;

float kalmanFilterX(float accAngle, float gyroRate);
float kalmanFilterY(float accAngle, float gyroRate);


void  INThandler(int sig)
{
        signal(sig, SIG_IGN);
        exit(0);
}

int mymillis()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
    return (diff<0);
}

int main(int argc, char *argv[])
{

	float rate_gyr_y = 0.0;   // [deg/s]
	float rate_gyr_x = 0.0;    // [deg/s]
	float rate_gyr_z = 0.0;     // [deg/s]

	int  accRaw[3];
	int  magRaw[3];
	int  gyrRaw[3];



	float gyroXangle = 0.0;
	float gyroYangle = 0.0;
	float gyroZangle = 0.0;
	float AccYangle = 0.0;
	float AccXangle = 0.0;
	float CFangleX = 0.0;
	float CFangleY = 0.0;

	int startInt  = mymillis();
	struct  timeval tvBegin, tvEnd,tvDiff;


	signal(SIGINT, INThandler);

	detectIMU();
	enableIMU();

	gettimeofday(&tvBegin, NULL);


	while(1)
	{
	startInt = mymillis();


	//read ACC and GYR data
	readACC(accRaw);
	readGYR(gyrRaw);

	//Convert Gyro raw to degrees per second
	rate_gyr_x = (float) gyrRaw[0]  * G_GAIN;
	rate_gyr_y = (float) gyrRaw[1]  * G_GAIN;
	rate_gyr_z = (float) gyrRaw[2]  * G_GAIN;



	//Calculate the angles from the gyro
	gyroXangle+=rate_gyr_x*DT;
	gyroYangle+=rate_gyr_y*DT;
	gyroZangle+=rate_gyr_z*DT;




	//Convert Accelerometer values to degrees
	AccXangle = (float) (atan2(accRaw[1],accRaw[2])+M_PI)*RAD_TO_DEG;
	AccYangle = (float) (atan2(accRaw[2],accRaw[0])+M_PI)*RAD_TO_DEG;






	//Change the rotation value of the accelerometer to -/+ 180 and move the Y axis '0' point to up.
	//Two different pieces of code are used depending on how your IMU is mounted.
	//If IMU is upside down
	/*
	if (AccXangle >180)
		AccXangle -= (float)360.0;

	AccYangle-=90;
	if (AccYangle >180)
		AccYangle -= (float)360.0;
	*/

	//If IMU is up the correct way, use these lines
	AccXangle -= (float)180.0;
	if (AccYangle > 90)
		AccYangle -= (float)270;
	else
		AccYangle += (float)90;

	//Kalman Filter
	float kalmanX = kalmanFilterX(AccXangle, rate_gyr_x);
	float kalmanY = kalmanFilterY(AccYangle, rate_gyr_y);
	printf ("\033[22;31mkalmanX %7.3f  \033[22;36mkalmanY %7.3f\t\e[m",kalmanX,kalmanY);

	//Complementary filter used to combine the accelerometer and gyro values.
	CFangleX=AA*(CFangleX+rate_gyr_x*DT) +(1 - AA) * AccXangle;
	CFangleY=AA*(CFangleY+rate_gyr_y*DT) +(1 - AA) * AccYangle;


	printf ("GyroX  %7.3f \t AccXangle \e[m %7.3f \t \033[22;31mCFangleX %7.3f\033[0m\t GyroY  %7.3f \t AccYangle %7.3f \t \033[22;36mCFangleY %7.3f\t\033[0m\n",gyroXangle,AccXangle,CFangleX,gyroYangle,AccYangle,CFangleY);

	//Each loop should be at least 20ms.
	while(mymillis() - startInt < (DT*1000))
	{
		usleep(100);
	}

	printf("Loop Time %d\t", mymillis()- startInt);
    }
}





float kalmanFilterX(float accAngle, float gyroRate){
	float  y, S;
	float K_0, K_1;


	KFangleX += DT * (gyroRate - x_bias);

	XP_00 +=  - DT * (XP_10 + XP_01) + Q_angle * DT;
	XP_01 +=  - DT * XP_11;
	XP_10 +=  - DT * XP_11;
	XP_11 +=  + Q_gyro * DT;

	y = accAngle - KFangleX;
	S = XP_00 + R_angle;
	K_0 = XP_00 / S;
	K_1 = XP_10 / S;

	KFangleX +=  K_0 * y;
	x_bias  +=  K_1 * y;
	XP_00 -= K_0 * XP_00;
	XP_01 -= K_0 * XP_01;
	XP_10 -= K_1 * XP_00;
	XP_11 -= K_1 * XP_01;

	return KFangleX;
}


float kalmanFilterY(float accAngle, float gyroRate){
	float  y, S;
	float K_0, K_1;


	KFangleY += DT * (gyroRate - y_bias);

	YP_00 +=  - DT * (YP_10 + YP_01) + Q_angle * DT;
	YP_01 +=  - DT * YP_11;
	YP_10 +=  - DT * YP_11;
	YP_11 +=  + Q_gyro * DT;

	y = accAngle - KFangleY;
	S = YP_00 + R_angle;
	K_0 = YP_00 / S;
	K_1 = YP_10 / S;

	KFangleY +=  K_0 * y;
	y_bias  +=  K_1 * y;
	YP_00 -= K_0 * YP_00;
	YP_01 -= K_0 * YP_01;
	YP_10 -= K_1 * YP_00;
	YP_11 -= K_1 * YP_01;

	return KFangleY;
}

