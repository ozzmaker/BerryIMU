/*This file can be used to test the magnetometer LIS3MDL.  
  This chip is found on the BerryIMUv3 and BerryGPS-IMUv4.
  For accurate results, the LIS3MDL must be as far away as possible
  from magnetic sources.
  
  Feel free to do whatever you like with this code
  Distributed as-is; no warranty is given.*/

#include <stdint.h>
#include "i2c-dev.h"
#include "LIS3MDL.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int file;


void  readBlock(uint8_t command, uint8_t size, uint8_t *data)
{
    int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    if (result != size){
                printf("Failed to read block from I2C.");
                exit(1);
        }
}



void readMAG(int  m[])
{
        uint8_t block[6];

        readBlock(LIS3MDL_OUT_X_L, sizeof(block), block);


        // Combine readings for each axis.
        m[0] = (int16_t)(block[0] | block[1] << 8);
        m[1] = (int16_t)(block[2] | block[3] << 8);
        m[2] = (int16_t)(block[4] | block[5] << 8);

}

void writeMagReg(uint8_t reg, uint8_t value)
{
        int result = i2c_smbus_write_byte_data(file, reg, value);
        if (result == -1){
                printf("Failed to write byte to I2C Mag.");
                exit(1);
        }
}

int main(int argc, char *argv[])

{
        //open I2C
        __u16 block[I2C_SMBUS_BLOCK_MAX];

        int res, bus,  size;

        char filename[20];
        sprintf(filename, "/dev/i2c-%d", 1);
        file = open(filename, O_RDWR);
        if (file<0) {
                printf("Unable to open I2C bus!");
                        exit(1);
        }
        if (ioctl(file, I2C_SLAVE, LIS3MDL_ADDRESS) < 0) {
                 printf("Failed to select I2C device.");
        }


        //Initialise sensor, turn on sensor, FS=12Gauss, continuous measurment mode, ODR = 80Hz
        writeMagReg(LIS3MDL_CTRL_REG1, 0b00011100);
        writeMagReg(LIS3MDL_CTRL_REG2, 0b00101000);
        usleep(50000);
        writeMagReg(LIS3MDL_CTRL_REG3, 0b00000000);
        int data[3];
        int i;
        int j;
        float val_st_off[] = {0,0,0};
        float val_st_on[] = {0,0,0};

        //Read all axis once and then discard
        readMAG(data);
        //Read all axis 5 times, convert to mg
        for( i=0;i<5;i++){
                usleep(10000);
                readMAG(data);

                for (j = 0; j < 3; j++)
                      val_st_off[j] += (float) data[j]/2281.0f;
        }
        /* Calculate the mg average values */
        for (i = 0; i < 3; i++)
                val_st_off[i] /= 5;

        //Enable self test
        writeMagReg(LIS3MDL_CTRL_REG1, 0b00011101);
        usleep(100000);
    //Read all axis once and then discard
    readMAG(data);

    //Read all axis 5 times, convert to mg
    for( i=0;i<5;i++){
                usleep(10000);
        readMAG(data);

                for (j = 0; j < 3; j++)
                      val_st_on[j] += (float) data[j]/2281.0f;
        }

    /* Calculate the mg average values */
    for (i = 0; i < 3; i++)
                val_st_on[i] /= 5;
    //printf("1st results Averaged  X = %f Y = %f Z = %f\n", val_st_on[0],val_st_on[1],val_st_on[2]);

        printf(" ---------------------------RESULTS------------------------------ \n");
        printf("| Axis | ST min [gauss] | ST max [gauss] |   Self test results  |\n");
        printf("-----------------------------------------------------------------\n");
        printf("|   X  |       1.0      |       3.0      |\t%f \t|\n",fabs( val_st_on[0]  -  val_st_off[0]));
        printf("|   Y  |       1.0      |       3.0      |\t%f \t|\n",fabs( val_st_on[1]  -  val_st_off[2]));
        printf("|   Z  |       0.1      |       1.0      |\t%f \t|\n",fabs( val_st_on[2]  -  val_st_off[2]));
        printf("-----------------------------------------------------------------\n");
        printf("The test results in the last column need to be between the min and max values for the that axis\n");
        writeMagReg(LIS3MDL_CTRL_REG1, 0b00011100); //Disable self test
        writeMagReg(LIS3MDL_CTRL_REG3, 0b00000011);  //Power down mode
}




