/*
        A simple program that demonstrates how to program a magnetometer
	on the Raspberry Pi.
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
#include "LSM303D.h"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int file;

void writeMagReg(uint8_t reg, uint8_t value);
void readBlock(uint8_t command, uint8_t size, uint8_t *data);
void readMAG(int * m);




int main(int argc, char *argv[])

{
	char filename[20];
	int magRaw[3];

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


	//Enable the magnetometer
	writeMagReg( LSM303D_CTRL5, 0xF0);   // Temp enable, M data rate = 50Hz
	writeMagReg( LSM303D_CTRL6, 0x60);   // +/-12gauss
	writeMagReg( LSM303D_CTRL7, 0x00);   // Continuous-conversion mode

	while(1)
	{
		readMAG(magRaw);
		printf("magRaw X %i\t magRaw Y %i\t MagRaw Z %i \n", magRaw[0],magRaw[1],magRaw[2]);

		//Only needed if the heading value does not increase when the magnetometer is rotated clockwise
		magRaw[1] = -magRaw[1];

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



void writeMagReg(uint8_t reg, uint8_t value)
{
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

void readMAG(int  *m)
{
        uint8_t block[6];

        readBlock(0x80 | LSM303D_OUT_X_L_M, sizeof(block), block);

        *m = (int16_t)(block[1] | block[0] << 8);
        *(m+1) = (int16_t)(block[3] | block[2] << 8) ;
        *(m+2) = (int16_t)(block[5] | block[4] << 8) ;

}






