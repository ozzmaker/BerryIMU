/*
    A simple program that demonstrates how to program a magnetometer
    on the Raspberry Pi.

    http://ozzmaker.com/compass1

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

int file;



int main(int argc, char *argv[])

{

	int magRaw[3];

	detectIMU();
	enableIMU();

	while(1){
		readMAG(magRaw);
		printf("magRaw X %i    \tmagRaw Y %i \tMagRaw Z %i \n", magRaw[0],magRaw[1],magRaw[2]);

		//Compute heading
		float heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;

		//Convert heading to 0 - 360
		if(heading < 0)
			heading += 360;

		printf("heading %7.3f \t ", heading);

		//Sleep for 0.25ms
		usleep(25000);

	}

}




