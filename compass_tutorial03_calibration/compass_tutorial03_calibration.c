/*
    http://ozzmaker.com/compass3

    The BerryIMUv1, BerryIMUv2 and BerryIMUv3 are supported.

    Feel free to do whatever you like with this code.
    Distributed as-is; no warranty is given.

    http://ozzmaker.com/
*/



#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "IMU.c"


int magXmax = -32767;
int magYmax = -32767;
int magZmax = -32767;
int magXmin = 32767;
int magYmin = 32767;
int magZmin = 32767;

int file;



//When Ctrl-C is pressed, we need to print out the high and low values which would then be copied into the main program.
void  INThandler(int sig)
{
	signal(sig, SIG_IGN);
	printf("\n\n\nCopy the below definitions to the start of your compass program. \n");
	printf("\033[01;36m#define magXmax %i\n#define magYmax %i\n#define magZmax %i\n", magXmax,magYmax,magZmax);
	printf("\033[01;36m#define magXmin %i\n#define magYmin %i\n#define magZmin %i\n\n", magXmin,magYmin,magZmin);

	exit(0);
}



int main(int argc, char *argv[])

{

	int magRaw[3];

	detectIMU();
	enableIMU();



	signal(SIGINT, INThandler);

	while(1)
	{
		readMAG(magRaw);
		printf("magXmax %4i magYmax %4i magZmax %4i magXmin %4i magYmin %4i magZmin %4i\n", magXmax,magYmax,magZmax,magXmin,magYmin,magZmin);

		if (magRaw[0] > magXmax) magXmax = magRaw[0];
		if (magRaw[1] > magYmax) magYmax = magRaw[1];
		if (magRaw[2] > magZmax) magZmax = magRaw[2];

		if (magRaw[0] < magXmin) magXmin = magRaw[0];
		if (magRaw[1] < magYmin) magYmin = magRaw[1];
		if (magRaw[2] < magZmin) magZmin = magRaw[2];

		//Sleep for 0.25ms
		usleep(25000);
	}

}





