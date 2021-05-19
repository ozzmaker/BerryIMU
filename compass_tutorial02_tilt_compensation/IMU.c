#include <stdint.h>
#include "i2c-dev.h"
#include "LSM9DS0.h"
#include "LSM9DS1.h"
#include "LSM6DSL.h"
#include "LIS3MDL.h"


int file;
int BerryIMUversion = 99;

void  readBlock(uint8_t command, uint8_t size, uint8_t *data)
{
    int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    if (result != size){
		printf("Failed to read block from I2C.");
		exit(1);
	}
}

void selectDevice(int file, int addr)
{
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		 printf("Failed to select I2C device.");
	}
}


void readACC(int  a[])
{
	uint8_t block[6];
	if (BerryIMUversion == 1){
		selectDevice(file,LSM9DS0_ACC_ADDRESS);
		readBlock(0x80 |  LSM9DS0_OUT_X_L_A, sizeof(block), block);
	}
	else if (BerryIMUversion == 2){
		selectDevice(file,LSM9DS1_ACC_ADDRESS);
		readBlock(LSM9DS1_OUT_X_L_XL, sizeof(block), block);       
	}
	else if (BerryIMUversion == 3){
		selectDevice(file,LSM6DSL_ADDRESS);
		readBlock(LSM6DSL_OUTX_L_XL, sizeof(block), block);    
	}
	// Combine readings for each axis.
	a[0] = (int16_t)(block[0] | block[1] << 8);
	a[1] = (int16_t)(block[2] | block[3] << 8);
	a[2] = (int16_t)(block[4] | block[5] << 8);
}


void readMAG(int  m[])
{
	uint8_t block[6];
    if (BerryIMUversion == 1){
		selectDevice(file,LSM9DS0_MAG_ADDRESS);
		readBlock(0x80 |  LSM9DS0_OUT_X_L_M, sizeof(block), block);
	}
	else if (BerryIMUversion == 2){
		selectDevice(file,LSM9DS1_MAG_ADDRESS);
		readBlock(LSM9DS1_OUT_X_L_M, sizeof(block), block);    
	}
	else if (BerryIMUversion == 3){
		selectDevice(file,LIS3MDL_ADDRESS);
		readBlock(LIS3MDL_OUT_X_L, sizeof(block), block);    
	}

	// Combine readings for each axis.
	m[0] = (int16_t)(block[0] | block[1] << 8);
	m[1] = (int16_t)(block[2] | block[3] << 8);
	m[2] = (int16_t)(block[4] | block[5] << 8);

}

void readGYR(int g[])
{
	uint8_t block[6];
    if (BerryIMUversion == 1){
		selectDevice(file,LSM9DS0_GYR_ADDRESS);
		readBlock(0x80 |  LSM9DS0_OUT_X_L_G, sizeof(block), block);
	}
	else if (BerryIMUversion == 2){
		selectDevice(file,LSM9DS1_GYR_ADDRESS);
		readBlock(LSM9DS1_OUT_X_L_G, sizeof(block), block);    
	}
	else if (BerryIMUversion == 3){
		selectDevice(file,LSM6DSL_ADDRESS);
		readBlock(LSM6DSL_OUTX_L_G, sizeof(block), block);   
	}

	// Combine readings for each axis.
	g[0] = (int16_t)(block[0] | block[1] << 8);
	g[1] = (int16_t)(block[2] | block[3] << 8);
	g[2] = (int16_t)(block[4] | block[5] << 8);
}


void writeAccReg(uint8_t reg, uint8_t value)
{
	if (BerryIMUversion == 1)
		selectDevice(file,LSM9DS0_ACC_ADDRESS);
	else if (BerryIMUversion == 2)
		selectDevice(file,LSM9DS1_ACC_ADDRESS);
	else if (BerryIMUversion == 3)
		selectDevice(file,LSM6DSL_ADDRESS);

	int result = i2c_smbus_write_byte_data(file, reg, value);
	if (result == -1){
		printf ("Failed to write byte to I2C Acc.");
        exit(1);
    }
}

void writeMagReg(uint8_t reg, uint8_t value)
{
	if (BerryIMUversion == 1)
		selectDevice(file,LSM9DS0_MAG_ADDRESS);
	else if (BerryIMUversion == 2)
		selectDevice(file,LSM9DS1_MAG_ADDRESS);
	else if (BerryIMUversion == 3)
		selectDevice(file,LIS3MDL_ADDRESS);

	int result = i2c_smbus_write_byte_data(file, reg, value);
	if (result == -1){
		printf("Failed to write byte to I2C Mag.");
		exit(1);
	}
}


void writeGyrReg(uint8_t reg, uint8_t value)
{
	if (BerryIMUversion == 1)
		selectDevice(file,LSM9DS0_GYR_ADDRESS);
	else if (BerryIMUversion == 2)
		selectDevice(file,LSM9DS1_GYR_ADDRESS);
	else if (BerryIMUversion == 3)
		selectDevice(file,LSM6DSL_ADDRESS);

	int result = i2c_smbus_write_byte_data(file, reg, value);
	if (result == -1){
		printf("Failed to write byte to I2C Gyr.");
		exit(1);
	}
}



void detectIMU()
{

	__u16 block[I2C_SMBUS_BLOCK_MAX];

	int res, bus,  size;


	char filename[20];
	sprintf(filename, "/dev/i2c-%d", 1);
	file = open(filename, O_RDWR);
	if (file<0) {
		printf("Unable to open I2C bus!");
			exit(1);
	}

	//Detect if BerryIMUv1 (Which uses a LSM9DS0) is connected
	selectDevice(file,LSM9DS0_ACC_ADDRESS);
	int LSM9DS0_WHO_XM_response = i2c_smbus_read_byte_data(file, LSM9DS0_WHO_AM_I_XM);

	selectDevice(file,LSM9DS0_GYR_ADDRESS);	
	int LSM9DS0_WHO_G_response = i2c_smbus_read_byte_data(file, LSM9DS0_WHO_AM_I_G);

	if (LSM9DS0_WHO_G_response == 0xd4 && LSM9DS0_WHO_XM_response == 0x49){
		printf ("\n\n\n#####   BerryIMUv1/LSM9DS0  DETECTED    #####\n\n");
		BerryIMUversion = 1;
	}




	//Detect if BerryIMUv2 (Which uses a LSM9DS1) is connected
	selectDevice(file,LSM9DS1_MAG_ADDRESS);
	int LSM9DS1_WHO_M_response = i2c_smbus_read_byte_data(file, LSM9DS1_WHO_AM_I_M);

	selectDevice(file,LSM9DS1_GYR_ADDRESS);	
	int LSM9DS1_WHO_XG_response = i2c_smbus_read_byte_data(file, LSM9DS1_WHO_AM_I_XG);

    if (LSM9DS1_WHO_XG_response == 0x68 && LSM9DS1_WHO_M_response == 0x3d){
		printf ("\n\n\n#####   BerryIMUv2/LSM9DS1  DETECTED    #####\n\n");
		BerryIMUversion = 2;
	}

	//Detect if BerryIMUv3 (Which uses a LSM6DSL and LIS3MDL) is connected
	selectDevice(file,LSM6DSL_ADDRESS);
	int LSM6DSL_WHO_M_response = i2c_smbus_read_byte_data(file, LSM6DSL_WHO_AM_I);

	selectDevice(file,LIS3MDL_ADDRESS);	
	int LIS3MDL_WHO_XG_response = i2c_smbus_read_byte_data(file, LIS3MDL_WHO_AM_I);

	if ( LSM6DSL_WHO_M_response == 0x6A && LIS3MDL_WHO_XG_response == 0x3D){
		printf ("\n\n\n#####   BerryIMUv3  DETECTED    #####\n\n");
		BerryIMUversion = 3;
	}

	sleep(1);
	if (BerryIMUversion == 99){
		printf ("NO IMU DETECTED\n");
		exit(1);
	}
}




void enableIMU()
{

	if (BerryIMUversion == 1){//For BerryIMUv1
		// Enable Gyroscope
		writeGyrReg(LSM9DS0_CTRL_REG1_G, 0b00001111); // Normal power mode, all axes enabled
		writeGyrReg(LSM9DS0_CTRL_REG4_G, 0b00110000); // Continuos update, 2000 dps full scale

		// Enable accelerometer.
		writeAccReg(LSM9DS0_CTRL_REG1_XM, 0b01100111); //  z,y,x axis enabled, continuous update,  100Hz data rate
		writeAccReg(LSM9DS0_CTRL_REG2_XM, 0b00100000); // +/- 16G full scale

		//Enable  magnetometer
		writeMagReg(LSM9DS0_CTRL_REG5_XM, 0b11110000); // Temp enable, M data rate = 50Hz
		writeMagReg(LSM9DS0_CTRL_REG6_XM, 0b01100000); // +/-12gauss
		writeMagReg(LSM9DS0_CTRL_REG7_XM, 0b00000000); // Continuous-conversion mode

	}

	if (BerryIMUversion == 2){//For BerryIMUv2      
		// Enable gyroscope
		writeGyrReg(LSM9DS1_CTRL_REG4,0b00111000);      // z, y, x axis enabled for gyro
		writeGyrReg(LSM9DS1_CTRL_REG1_G,0b10111000);    // Gyro ODR = 476Hz, 2000 dps
		writeGyrReg(LSM9DS1_ORIENT_CFG_G,0b10111000);   // Swap orientation 

		// Enable the accelerometer
		writeAccReg(LSM9DS1_CTRL_REG5_XL,0b00111000);   // z, y, x axis enabled for accelerometer
		writeAccReg(LSM9DS1_CTRL_REG6_XL,0b00101000);   // +/- 16g

		//Enable the magnetometer
		writeMagReg(LSM9DS1_CTRL_REG1_M, 0b10011100);   // Temp compensation enabled,Low power mode mode,80Hz ODR
		writeMagReg(LSM9DS1_CTRL_REG2_M, 0b01000000);   // +/-12gauss
		writeMagReg(LSM9DS1_CTRL_REG3_M, 0b00000000);   // continuos update
		writeMagReg(LSM9DS1_CTRL_REG4_M, 0b00000000);   // lower power mode for Z axis
	}
	if (BerryIMUversion == 3){//For BerryIMUv3
		//Enable  gyroscope
		writeGyrReg(LSM6DSL_CTRL2_G,0b10011100);        // ODR 3.3 kHz, 2000 dps

		// Enable the accelerometer
		writeAccReg(LSM6DSL_CTRL1_XL,0b10011111);       // ODR 3.33 kHz, +/- 8g , BW = 400hz
		writeAccReg(LSM6DSL_CTRL8_XL,0b11001000);       // Low pass filter enabled, BW9, composite filter
		writeAccReg(LSM6DSL_CTRL3_C,0b01000100);        // Enable Block Data update, increment during multi byte read

		//Enable  magnetometer
		writeMagReg(LIS3MDL_CTRL_REG1, 0b11011100);     // Temp sesnor enabled, High performance, ODR 80 Hz, FAST ODR disabled and Selft test disabled.
		writeMagReg(LIS3MDL_CTRL_REG2, 0b00100000);     // +/- 8 gauss
		writeMagReg(LIS3MDL_CTRL_REG3, 0b00000000);     // Continuous-conversion mode
	}


}

