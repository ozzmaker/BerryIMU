/*
    This program  demonstrates how to program a magnetometer
    on the Raspberry Pi and includes tilt compensation.
    http://ozzmaker.com/compass2

    If the tilt compensation isnt working, try calibrating the compass first. 
    Instructions here ttp://ozzmaker.com/compass3

    The BerryIMUv1, BerryIMUv2 and BerryIMUv3 are supported.

    Feel free to do whatever you like with this code.
    Distributed as-is; no warranty is given.

    http://ozzmaker.com/
*/





#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "IMU.c"



int main(int argc, char *argv[])

{

	int magRaw[3];
	int accRaw[3];
	float accXnorm,accYnorm,pitch,roll,magXcomp,magYcomp;


	detectIMU();
	enableIMU();


	while(1)
	{
		readMAG(magRaw);
		readACC(accRaw);

		//If your IMU is upside down(Skull logo facing up), comment out the two lines below which will correct the tilt calculation
		//accRaw[0] = -accRaw[0];
		//accRaw[1] = -accRaw[1];

		//Compute heading
		float heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;

		//Convert heading to 0 - 360
		if(heading < 0)
			heading += 360;

		printf("heading %7.3f \t ", heading);

		//Normalize accelerometer raw values.
		accXnorm = accRaw[0]/sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);
		accYnorm = accRaw[1]/sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);

		//Calculate pitch and roll
		pitch = asin(accXnorm);
		roll = -asin(accYnorm/cos(pitch));

		//Calculate the new tilt compensated values
		//The compass and accelerometer are orientated differently on the the BerryIMUv1, v2 and v3.
		//needs to be taken into consideration when performing the calculations
		//X compensation
		if(BerryIMUversion == 1 || BerryIMUversion == 3)
			magXcomp = magRaw[0]*cos(pitch)+magRaw[2]*sin(pitch);
		else if (BerryIMUversion == 2)
			magXcomp = magRaw[0]*cos(pitch)-magRaw[2]*sin(pitch);

		//Y compensation
		if(BerryIMUversion == 1 || BerryIMUversion == 3)
			magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)-magRaw[2]*sin(roll)*cos(pitch); // LSM9DS0
		else if (BerryIMUversion == 2)
			magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)+magRaw[2]*sin(roll)*cos(pitch); // LSM9DS1

		//Calculate heading
		heading = 180*atan2(magYcomp,magXcomp)/M_PI;

		//Convert heading to 0 - 360
		if(heading < 0)
			heading += 360;


		printf("Compensated  Heading %7.3f  \n", heading);


		//Sleep for 25ms
		usleep(25000);

	}

}



