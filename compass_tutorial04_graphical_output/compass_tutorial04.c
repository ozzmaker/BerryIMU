/*
        A simple program that demonstrates how to program a magnetometer
	on the Raspberry Pi and includes tilt compensation.
	http://ozzmaker.com/2014/12/01/compass1


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
#include <stdint.h>
#include "LSM9DS0.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>

#include "sdl.c"


#define magXmax 419
#define magYmax 683
#define magZmax 528
#define magXmin -658
#define magYmin -397
#define magZmin -472

#define MAG_LPF_FACTOR  0.4
#define ACC_LPF_FACTOR  0.1

int file;

void writeMagReg(uint8_t reg, uint8_t value);
void writeAccReg(uint8_t reg, uint8_t value);
void readBlock(uint8_t command, uint8_t size, uint8_t *data);
void readMAG(int * m);
void readACC(int * a);


void  INThandler(int sig)
{
	closeSDL();
        signal(sig, SIG_IGN);

        exit(0);
}




int main(int argc, char *argv[])

{
	char filename[20];
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



	//Open the i2c bus
	sprintf(filename, "/dev/i2c-%d", 1);
	file = open(filename, O_RDWR);
	if (file<0) {
        	printf("Unable to open I2C bus!");
                exit(1);
	}


	//Select the magnetomoter
	if (ioctl(file, I2C_SLAVE, MAG_ADDRESS) < 0) {
                printf("Error: Could not select magnetometer\n");
        }



	// Enable accelerometer.
        writeAccReg(CTRL_REG1_XM, 0b01100111); //  z,y,x axis enabled, continuos update,  100Hz data rate
        writeAccReg(CTRL_REG2_XM, 0b00100000); // +/- 16G full scale

        //Enable the magnetometer
        writeMagReg( CTRL_REG5_XM, 0b11110000);   // Temp enable, M data rate = 50Hz
        writeMagReg( CTRL_REG6_XM, 0b01100000);   // +/-12gauss
        writeMagReg( CTRL_REG7_XM, 0b00000000);   // Continuous-conversion mode

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
		magXcomp = magRaw[0]*cos(pitch)+magRaw[02]*sin(pitch);
		magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)-magRaw[2]*sin(roll)*cos(pitch);


		//Calculate heading
		heading = 180*atan2(magYcomp,magXcomp)/M_PI;

                //Convert heading to 0 - 360
		if(heading < 0)
		      heading += 360;
		graphics(heading);


		printf("Compensated  Heading %7.3f  \n", heading);
	}

}

void selectDevice(int file, int addr)
{
        if (ioctl(file, I2C_SLAVE, addr) < 0) {
		 printf("Failed to select I2C device.");
        }
}


void writeAccReg(uint8_t reg, uint8_t value)
{
  selectDevice(file,ACC_ADDRESS);

  int result = i2c_smbus_write_byte_data(file, reg, value);
    if (result == -1)
    {
        printf ("Failed to write byte to I2C Mag.");
        exit(1);
    }
}



void writeMagReg(uint8_t reg, uint8_t value)
{
  selectDevice(file,MAG_ADDRESS);
  int result = i2c_smbus_write_byte_data(file, reg, value);
    if (result == -1)
    {
        printf ("Failed to write byte to I2C Mag.");
        exit(1);
    }
}

void  readBlock(uint8_t command, uint8_t size, uint8_t *data)
{
    int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    if (result != size)
    {
       printf("Failed to read block from I2C.");
        exit(1);
    }
}


void readACC(int  *a)
{
        uint8_t block[6];
        selectDevice(file,ACC_ADDRESS);
                readBlock(0x80 | OUT_X_L_A, sizeof(block), block);

        *a = (int16_t)(block[0] | block[1] << 8);
        *(a+1) = (int16_t)(block[2] | block[3] << 8);
        *(a+2) = (int16_t)(block[4] | block[5] << 8);

}

void readMAG(int  *m)
{
        uint8_t block[6];

        readBlock(0x80 | OUT_X_L_M, sizeof(block), block);

        *m = (int16_t)(block[0] | block[1] << 8);
        *(m+1) = (int16_t)(block[2] | block[3] << 8);
        *(m+2) = (int16_t)(block[4] | block[5] << 8);

}


