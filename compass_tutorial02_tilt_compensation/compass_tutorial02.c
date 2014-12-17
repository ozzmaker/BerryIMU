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
#include "LSM303.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int file;

void writeMagReg(uint8_t reg, uint8_t value);
void writeAccReg(uint8_t reg, uint8_t value);
void readBlock(uint8_t command, uint8_t size, uint8_t *data);
void readMAG(int * m);
void readACC(int * a);




int main(int argc, char *argv[])

{
	char filename[20];
	int magRaw[3];
	int accRaw[3];
	float accXnorm,accYnorm,pitch,roll,magXcomp,magYcomp;




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
        writeAccReg(LSM303_CTRL_REG1_A, 0b01010111); //  z,y,x axis enabled , 100Hz data rate
        writeAccReg(LSM303_CTRL_REG4_A, 0b00101000); // +/- 8G full scale: FS = 10 on DLHC, high resolution output mode

	 // Enable magnetometer
        writeMagReg(LSM303_MR_REG_M, 0b00000000);  // enable magnometer


	//data output rate 220(Hz)
	writeMagReg(LSM303_CRA_REG_M, 0b00011100); // enable tempereture sensor

	while(1)
	{
		readMAG(magRaw);
		readACC(accRaw);


		//Compute heading
	        float heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;

		//Convert heading to 0 - 360
        	if(heading < 0)
	  	      heading += 360;

		printf("heading %7.3f \t ", heading);

		//Normalize accelerometer raw values.
                accXnorm = accRaw[0]/sqrt(accRaw[0]* accRaw[0]+ accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);
                accYnorm =accRaw[1]/sqrt(accRaw[0] *accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);

		//Calculate pitch and roll
		pitch = asin(accXnorm);
		roll = asin(accYnorm/cos(pitch));

		//Calculate the new tilt compensated values
		magXcomp = magRaw[0]*cos(pitch)+magRaw[02]*sin(pitch);
		magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)-magRaw[2]*sin(roll)*cos(pitch);


		//Calculate heading
		heading = 180*atan2(magYcomp,magXcomp)/M_PI;

                //Convert heading to 0 - 360
		if(heading < 0)
		      heading += 360;


		printf("Compensated  Heading %7.3f  \n", heading);


		//Sleep for 0.25ms
		usleep(25000);

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
                readBlock(0x80 | LSM303_OUT_X_L_A, sizeof(block), block);

        *a = (int16_t)(block[0] | block[1] << 8) >> 4;
        *(a+1) = (int16_t)(block[2] | block[3] << 8) >> 4;
        *(a+2) = (int16_t)(block[4] | block[5] << 8) >> 4;

}
void readMAG(int  *m)
{
        uint8_t block[6];
        selectDevice(file,MAG_ADDRESS);
        // DLHC: register address order is X,Z,Y with high bytes first
        readBlock(0x80 | LSM303_OUT_X_H_M, sizeof(block), block);

        *m = (int16_t)(block[1] | block[0] << 8);
        *(m+1) = (int16_t)(block[5] | block[4] << 8) ;
        *(m+2) = (int16_t)(block[3] | block[2] << 8) ;

}

