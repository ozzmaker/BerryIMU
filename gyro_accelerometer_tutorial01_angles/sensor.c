/*
        This program  reads the angles from the acceleromter and gyrscope
        on a BerryIMU connected to a Raspberry Pi.
        http://ozzmaker.com/


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
#include "linux/i2c-dev.h"
#include "LSM9DS0.h"

int imu_file = 0;

void readBlock(uint8_t command, uint8_t size, uint8_t * data)
{
    int result = i2c_smbus_read_i2c_block_data(imu_file, command, size, data);
    if (result != size) {
		printf("Failed to read block from I2C.");
		exit(1);
    }
}

void selectDevice(int file, int address)
{
	if( ioctl(file, I2C_SLAVE, address) < 0)
		printf("Failed to select I2C device.");
}

void readAccelerometer(int16_t * data)
{
	uint8_t block[6];
	selectDevice(imu_file, ACC_ADDRESS);
	readBlock(0x80 | OUT_X_L_A, sizeof(block), block);
	
	*(data    ) = (int16_t)(block[0] | block[1] << 8);
	*(data + 1) = (int16_t)(block[2] | block[3] << 8);
	*(data + 2) = (int16_t)(block[4] | block[5] << 8);
}

void readMagnetometer(int16_t * data)
{
	uint8_t block[6];
	selectDevice(imu_file, MAG_ADDRESS);
	readBlock(0x80 | OUT_X_L_M, sizeof(block), block);
	
	*(data    ) = (int16_t)(block[0] | block[1] << 8);
	*(data + 1) = (int16_t)(block[2] | block[3] << 8);
	*(data + 2) = (int16_t)(block[4] | block[5] << 8);
}

void readGyroscope(int16_t * data)
{
	uint8_t block[6];
	selectDevice(imu_file, GYR_ADDRESS);
	readBlock(0x80 | OUT_X_L_G, sizeof(block), block);

	*(data    ) = (int16_t)(block[0] | block[1] << 8);
	*(data + 1) = (int16_t)(block[2] | block[3] << 8);
	*(data + 2) = (int16_t)(block[4] | block[5] << 8);
}

void writeAccelerometer(uint8_t reg, uint8_t value)
{
	selectDevice(imu_file, ACC_ADDRESS);
	int result = i2c_smbus_write_byte_data(imu_file, reg, value);
	if (result == -1) {
		printf ("Failed to write byte to I2C Accelerometer.");
		exit(1);
    }
}

void writeMagnetometer(uint8_t reg, uint8_t value)
{
	selectDevice(imu_file, MAG_ADDRESS);
	int result = i2c_smbus_write_byte_data(imu_file, reg, value);
	if (result == -1) {
		printf("Failed to write byte to I2C Magnetometer.");
		exit(1);
    }
}

void writeGyroscope(uint8_t reg, uint8_t value)
{
	selectDevice(imu_file, GYR_ADDRESS);
	int result = i2c_smbus_write_byte_data(imu_file, reg, value);
	if (result == -1) {
        printf("Failed to write byte to I2C Gyroscope.");
        exit(1);
    }
}

void enableIMU()
{	
	char filename[32];

	sprintf(filename, "/dev/i2c-%d", 1);
	
	imu_file = open(filename, O_RDWR);
	
	if(imu_file < 0) {
		printf("Unable to open I2C bus!");
		exit(1);
	}
	
	// Enable accelerometer
	writeAccelerometer(CTRL_REG1_XM, 0b01100111); // enable X, Y, Z axes, set continuous update, 100 Hz data rate
	writeAccelerometer(CTRL_REG2_XM, 0b00100000); // +/- 16 G full scale
	
	// Enable magnetometer
	writeMagnetometer (CTRL_REG5_XM, 0b11110000); // Temp enable, M data rate = 50Hz
	writeMagnetometer (CTRL_REG6_XM, 0b01100000); // +/-12 gauss
	writeMagnetometer (CTRL_REG7_XM, 0b00000000); // Continuous-conversion mode
	
	// Enable gyroscope
	writeGyroscope    (CTRL_REG1_G, 0b00001111); // Normal power mode, all axes enabled
	writeGyroscope    (CTRL_REG4_G, 0b00110000); // Continuos update, 2000 dps full scale

}



