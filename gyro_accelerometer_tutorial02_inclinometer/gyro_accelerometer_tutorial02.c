/*

	This program is used to create an Inclinometer using a
	Raspberry Pi and an IMU

	The BerryIMUv1, BerryIMUv2 and BerryIMUv3 are supported.

	Feel free to do whatever you like with this code
	Distributed as-is; no warranty is given.

	http://ozzmaker.com/inclinometer-using-raspberry-pi-imu/

 
*/



#include <sys/time.h>
#include "SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_rotozoom.h"
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "IMU.c"


#define DT 0.02         // [s/loop] loop period.  0.02  = 20ms
#define AA 0.97         // complementary filter constant

#define A_GAIN 0.0573   // [deg/LSB]
#define G_GAIN 0.070    // [deg/s/LSB]
#define RAD_TO_DEG 57.29578
#define M_PI 3.14159265358979323846

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT  320

int graphics(float carRoll, float carPitch);
int startSDL();

SDL_Surface* screen = NULL;
SDL_Surface* inclinometerJeepFront = NULL;
SDL_Surface* inclinometerJeepSide = NULL;
SDL_Surface* inclinometerOverlay = NULL;
SDL_Surface* compatibleInclinometerJeepFront = NULL;
SDL_Surface* compatibleInclinometerJeepSide = NULL;
SDL_Surface* compatibleInclinometerOverlay = NULL;
SDL_Surface* rotationInclinometerJeepFront = NULL;
SDL_Surface* rotationInclinometerJeepSide = NULL;
SDL_Rect inclinometerJeepOverlayPosition;
SDL_Rect inclinometerJeepFrontPosition;
SDL_Rect inclinometerJeepSidePosition;

SDL_VideoInfo* videoInfo;



void  INThandler(int sig)
{
	SDL_FreeSurface(screen);
	SDL_Quit();
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

	int  acc_raw[3];
	int  mag_raw[3];
	int  gyr_raw[3];



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

	if(argc <=1)
		startSDL();
	else if (strcmp(argv[1],"nosdl")!= 0)
		startSDL();

	while(1){
	startInt = mymillis();


	//read ACC and GYR data
	readACC(acc_raw);
	readGYR(gyr_raw);

	//Convert Gyro raw to degrees per second
	rate_gyr_x = (float) gyr_raw[0] * G_GAIN;
	rate_gyr_y = (float) gyr_raw[1]  * G_GAIN;
	rate_gyr_z = (float) gyr_raw[2]  * G_GAIN;



	//Calculate the angles from the gyro
	gyroXangle+=rate_gyr_x*DT;
	gyroYangle+=rate_gyr_y*DT;
	gyroZangle+=rate_gyr_z*DT;




	//Convert Accelerometer values to degrees
	AccXangle = (float) (atan2(acc_raw[1],acc_raw[2])+M_PI)*RAD_TO_DEG;
	AccYangle = (float) (atan2(acc_raw[2],acc_raw[0])+M_PI)*RAD_TO_DEG;



	//Change the rotation value of the accelerometer to -/+ 180 and move the Y axis '0' point to up.
	//Two different pieces of code are used depending on how your IMU is mounted.
	//If IMU is upside down
	/*
	if (AccXangle >180)
		AccXangle -= (float)360.0;

	AccYangle-=90;
	if (AccYangle >180)
	A	ccYangle -= (float)360.0;
	*/

	//If IMU is up the correct way, use these lines
	AccXangle -= (float)180.0;
	if (AccYangle > 90)
		AccYangle -= (float)270;
	else
		AccYangle += (float)90;




	//Complementary filter used to combine the accelerometer and gyro values.
	CFangleX=AA*(CFangleX+rate_gyr_x*DT) +(1 - AA) * AccXangle;
	CFangleY=AA*(CFangleY+rate_gyr_y*DT) +(1 - AA) * AccYangle;


	printf ("   GyroX  %7.3f \t AccXangle \e[m %7.3f \t \033[22;31mCFangleX %7.3f\033[0m\t GyroY  %7.3f \t AccYangle %7.3f \t \033[22;36mCFangleY %7.3f\t\033[0m\n",gyroXangle,AccXangle,CFangleX,gyroYangle,AccYangle,CFangleY);


	if(argc <=1)
		graphics(CFangleX,CFangleY);
	else if (strcmp(argv[1],"nosdl")!= 0)
		graphics(CFangleX,CFangleY);


	//Each loop should be at least 20ms.
	while(mymillis() - startInt < (DT*1000)){
		usleep(100);
	}

	printf("Loop Time %d\t", mymillis()- startInt);
    }
}

int startSDL()
{

	//fb1 = small TFT.   fb0 = HDMI/RCA output
	//putenv("SDL_FBDEV=/dev/fb0");


	//Initialize  SDL and disable mouse
	SDL_Init(SDL_INIT_VIDEO);
	SDL_ShowCursor(SDL_DISABLE);

	//Get information about the current video device.  E.g. resolution and bits per pixal
	const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

	//Setup a Video mode.
	//screen = SDL_SetVideoMode(videoInfo->current_w, videoInfo->current_h, videoInfo->vfmt->BitsPerPixel, SDL_SWSURFACE );
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, videoInfo->vfmt->BitsPerPixel, SDL_SWSURFACE );

	if ( screen == NULL ) {
		fprintf(stderr, "Unable to setvideo: %s\n", SDL_GetError());
		exit(1);
	}

	//Load image.
	inclinometerJeepFront =  IMG_Load("inclinometerJeepFront.png");
	if (inclinometerJeepFront == NULL){
		printf("error loading JeepFront image\n");
		SDL_Quit();
		exit(1);
	}
	//Convert the image to a format we can use.
	compatibleInclinometerJeepFront = SDL_DisplayFormatAlpha(inclinometerJeepFront);

	//Load image.
	inclinometerJeepSide =  IMG_Load("inclinometerJeepSide.png");
	if (inclinometerJeepSide == NULL){
		 printf("error loading JeepSide image\n");
		SDL_Quit();
		exit(1);
	}
	//Convert the image to a format we can use.
	compatibleInclinometerJeepSide = SDL_DisplayFormatAlpha(inclinometerJeepSide);

	//Load image.
	inclinometerOverlay =  IMG_Load("inclinometerOverlay.png");
	if (inclinometerOverlay == NULL){
		 printf("error loading Overlay image\n");
		SDL_Quit();
		exit(1);
	}
	//Convert the image to a format we can use.
	compatibleInclinometerOverlay = SDL_DisplayFormatAlpha(inclinometerOverlay);

	//Position the overlay in the middle of the screen.   screen height minus the height of the overlay image.
	inclinometerJeepOverlayPosition.y = (SCREEN_HEIGHT/2)-(compatibleInclinometerOverlay->h/2);


}





int graphics(float carRoll, float carPitch)
{

	//Set all pixels to black to clear the last image.
	SDL_FillRect(screen,NULL,0x000000);

	///Position of both jeep images.
	inclinometerJeepFrontPosition.x = 30;
	inclinometerJeepFrontPosition.y = (SCREEN_HEIGHT/2)-((compatibleInclinometerJeepFront->h)/2);


	inclinometerJeepSidePosition.x = 262;
	inclinometerJeepSidePosition.y = (SCREEN_HEIGHT/2)-(compatibleInclinometerJeepFront->h/2);


	//Rotate the images based on pitch and roll.
	rotationInclinometerJeepSide = rotozoomSurface(compatibleInclinometerJeepSide, carPitch, 1.0, 0.0);
	rotationInclinometerJeepFront = rotozoomSurface(compatibleInclinometerJeepFront, carRoll, 1.0, 0.0);

	//Recenter pivot point.
	inclinometerJeepFrontPosition.x -= rotationInclinometerJeepFront->w/2-compatibleInclinometerJeepFront->w/2;
	inclinometerJeepFrontPosition.y -= rotationInclinometerJeepFront->h/2-compatibleInclinometerJeepFront->h/2;
	inclinometerJeepSidePosition.x -= rotationInclinometerJeepSide->w/2-compatibleInclinometerJeepSide->w/2;
	inclinometerJeepSidePosition.y -= rotationInclinometerJeepSide->h/2-compatibleInclinometerJeepSide->h/2;

	//Blit the three images to the surface.
	SDL_BlitSurface( compatibleInclinometerOverlay, NULL, screen, &inclinometerJeepOverlayPosition);
	SDL_BlitSurface( rotationInclinometerJeepFront, NULL, screen, &inclinometerJeepFrontPosition);
	SDL_BlitSurface( rotationInclinometerJeepSide, NULL, screen, &inclinometerJeepSidePosition);



	//Send the surface to the display.
	SDL_Flip(screen);

	//Free surfaces.
	SDL_FreeSurface(screen);
	SDL_FreeSurface(rotationInclinometerJeepFront);
	SDL_FreeSurface(rotationInclinometerJeepSide);
	return 0;

}
