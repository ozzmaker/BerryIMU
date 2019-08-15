/*
    A simple program that demonstrates how to program a magnetometer
    on the Raspberry Pi and includes tilt compensation.

    This program  demonstrates how to calibrate a magnetometer
    on the Raspberry Pi.
    http://ozzmaker.com/compass04


    Both the BerryIMUv1 and BerryIMUv2 are supported.

    Feel free to do whatever you like with this code.
    Distributed as-is; no warranty is given.

    http://ozzmaker.com/
*/

#include <stdint.h>
#include "LSM9DS0.h"
#include "LSM9DS1.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include "IMU.c"
#include "sdl.c"


//################# Compass Calibration values ############
// Use compass_tutorial03_calibration.c to get calibration values 
// Calibrating the compass isnt mandatory, however a calibrated 
// compass will result in a more accurate heading values.

#define magXmin 0.0
#define magYmin 0.0
#define magZmin 0.0
#define magXmax 0.0
#define magYmax 0.0
#define magZmax 0.0

/*Here is an example:
#define magXmin -1437
#define magYmin -1050
#define magZmin -1954
#define magXmax 896
#define magYmax 1592
#define magZmax 398
Dont use the above values, these are just an example.*/




#define MAG_LPF_FACTOR  0.4
#define ACC_LPF_FACTOR  0.1

int file;

void writeMagReg(uint8_t reg, uint8_t value);
void writeAccReg(uint8_t reg, uint8_t value);
void readBlock(uint8_t command, uint8_t size, uint8_t *data);



void  INThandler(int sig)
{
	closeSDL();
        signal(sig, SIG_IGN);

        exit(0);
}





int main(int argc, char *argv[])

{
	//char filename[20];
	int magRaw[3];
	int accRaw[3];
	float accXnorm,accYnorm,pitch,roll,magXcomp,magYcomp;
	float scaledMag[3];

	int oldXMagRawValue = 0;
	int oldYMagRawValue = 0;
	int oldZMagRawValue = 0;
	int oldXAccRawValue = 0;
	int oldYAccRawValue = 0;
	int oldZAccRawValue = 0;


	signal(SIGINT, INThandler);



	detectIMU();
	enableIMU();



	startSDL();


	while(1)
	{
		readMAG(magRaw);
		readACC(accRaw);

		//Apply low pass filter to reduce noise
		magRaw[0] =  magRaw[0]  * MAG_LPF_FACTOR + oldXMagRawValue*(1 - MAG_LPF_FACTOR);
		magRaw[1] =  magRaw[1]  * MAG_LPF_FACTOR + oldYMagRawValue*(1 - MAG_LPF_FACTOR);
		magRaw[2] =  magRaw[2]  * MAG_LPF_FACTOR + oldZMagRawValue*(1 - MAG_LPF_FACTOR);
		accRaw[0] =  accRaw[0]  * ACC_LPF_FACTOR + oldXAccRawValue*(1 - ACC_LPF_FACTOR);
		accRaw[1] =  accRaw[1]  * ACC_LPF_FACTOR + oldYAccRawValue*(1 - ACC_LPF_FACTOR);
		accRaw[2] =  accRaw[2]  * ACC_LPF_FACTOR + oldZAccRawValue*(1 - ACC_LPF_FACTOR);


		oldXMagRawValue = magRaw[0];
		oldYMagRawValue = magRaw[1];
		oldZMagRawValue = magRaw[2];
		oldXAccRawValue = accRaw[0];
		oldYAccRawValue = accRaw[1];
		oldZAccRawValue = accRaw[2];



		//Apply hard iron calibration
		magRaw[0] -= (magXmin + magXmax) /2 ;
		magRaw[1] -= (magYmin + magYmax) /2 ;
		magRaw[2] -= (magZmin + magZmax) /2 ;

		//Apply soft iron calibration
		scaledMag[0]  = (float)(magRaw[0] - magXmin) / (magXmax - magXmin) * 2 - 1;
		scaledMag[1]  = (float)(magRaw[1] - magYmin) / (magYmax - magYmin) * 2 - 1;
		scaledMag[2]  = (float)(magRaw[2] - magZmin) / (magZmax - magZmin) * 2 - 1;

		//If your IMU is upside down, comment out the two lines below which we correct the tilt calculation
		//accRaw[0] = -accRaw[0];
		//accRaw[1] = -accRaw[1];

		//Compute heading
		float heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;

		//Convert heading to 0 - 360
		if(heading < 0)
			heading += 360;


		//Normalize accelerometer raw values.
		accXnorm = accRaw[0]/sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);
		accYnorm = accRaw[1]/sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);

		//Calculate pitch and roll
		pitch = asin(accXnorm);
		roll = -asin(accYnorm/cos(pitch));


        //Calculate the new tilt compensated values
		magXcomp = magRaw[0]*cos(pitch)+magRaw[2]*sin(pitch);
		if(LSM9DS0)
			magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)-magRaw[2]*sin(roll)*cos(pitch); // LSM9DS0
		else
			magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)+magRaw[2]*sin(roll)*cos(pitch); // LSM9DS1

		//Calculate heading
		heading = 180*atan2(magYcomp,magXcomp)/M_PI;

		//Convert heading to 0 - 360
		if(heading < 0)
			heading += 360;
		graphics(heading);


		printf("Compensated  Heading %7.3f  \n", heading);
	}

}

