/*
    This program  demonstrates how to calibrate a magnetometer
    on the Raspberry Pi.
    http://ozzmaker.com/compass3

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


void readMAG(int * m);



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

int main(int argc, char *argv[])

{

	int magRaw[3];
	float scaledMag[3];


	detectIMU();
	enableIMU();



	while(1){
		readMAG(magRaw);

		//Apply hard iron calibration
		magRaw[0] -= (magXmin + magXmax) /2 ;
		magRaw[1] -= (magYmin + magYmax) /2 ;
		magRaw[2] -= (magZmin + magZmax) /2 ;

		//Apply soft iron calibration
		scaledMag[0]  = (float)(magRaw[0] - magXmin) / (magXmax - magXmin) * 2 - 1;
		scaledMag[1]  = (float)(magRaw[1] - magYmin) / (magYmax - magYmin) * 2 - 1;
		scaledMag[2]  = (float)(magRaw[2] - magZmin) / (magZmax - magZmin) * 2 - 1;


		printf("magRaw X %i\tmagRaw Y %i \tMagRaw Z %i \n", magRaw[0],magRaw[1],magRaw[2]);


		//Compute heading
		float heading = 180 * atan2(scaledMag[1],scaledMag[0])/M_PI;


		//Convert heading to 0 - 360
		if(heading < 0)
			heading += 360;


		//Local declination in mrads into radians
		float declination = 217.9 / 1000.0;

		//Add the declination correction to our current heading
		heading += declination * 180/M_PI;


		//Correct the heading if declination forces it over 360
		if ( heading > 360)
			heading -= 360;



		printf("heading %7.3f \t ", heading);

		//Sleep for 0.25ms
		usleep(25000);

	}

}









