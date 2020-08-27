#include <Arduino.h>
#include <Wire.h>
#include "LSM9DS0.h"
#include "LSM9DS1.h"
#include "LSM6DSL.h"
#include "LIS3MDL.h"



int BerryIMUversion = 99;

void writeTo(int device, byte address, byte val) {
   Wire.beginTransmission(device); //start transmission to device 
   Wire.write(address);        // send register address
   Wire.write(val);        // send value to write
   Wire.endTransmission(); //end transmission
}


void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  Wire.beginTransmission(device); //start transmission to device (initiate again)
  Wire.requestFrom(device, num);    // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}



void detectIMU(){
  //Detect which version of BerryIMU is connected using the 'who am i' register
  //BerryIMUv1 uses the LSM9DS0
  //BerryIMUv2 uses the LSM9DS1
  //BerryIMUv3 uses the LSM6DSL and LIS3MDL
  
  Wire.begin(); 
  byte LSM9DS0_WHO_AM_I_G_response;
  byte LSM9DS0_WHO_AM_I_XM_response;
  byte LSM9DS1_WHO_M_response;
  byte LSM9DS1_WHO_XG_response;
  byte LSM6DSL_WHO_AM_I_response;
  byte LIS3MDL_WHO_AM_I_response;

  
  byte WHOresponse[2];

  //Detect if BerryIMUv1 (Which uses a LSM9DS0) is connected
  readFrom(LSM9DS0_GYR_ADDRESS, LSM9DS0_WHO_AM_I_G,1,WHOresponse);
  LSM9DS0_WHO_AM_I_G_response = WHOresponse[0];
  
  readFrom(LSM9DS0_ACC_ADDRESS, LSM9DS0_WHO_AM_I_XM,1,WHOresponse);
  LSM9DS0_WHO_AM_I_XM_response = WHOresponse[0];

  if (LSM9DS0_WHO_AM_I_G_response == 0xD4 && LSM9DS0_WHO_AM_I_XM_response == 0x49){
    Serial.println("\n\n   BerryIMUv1(LSM9DS0) found \n\n");
    BerryIMUversion = 1;
  }

  
  //Detect if BerryIMUv2 (Which uses a LSM9DS1) is connected
  readFrom(LSM9DS1_MAG_ADDRESS, LSM9DS1_WHO_AM_I_M,1,WHOresponse);
  LSM9DS1_WHO_M_response = WHOresponse[0];
  
  readFrom(LSM9DS1_GYR_ADDRESS, LSM9DS1_WHO_AM_I_XG,1,WHOresponse);
  LSM9DS1_WHO_XG_response = WHOresponse[0];

  if (LSM9DS1_WHO_XG_response == 0x68 && LSM9DS1_WHO_M_response == 0x3D){
    Serial.println("\n\n   BerryIMUv2(LSM9DS1) found \n\n");
    BerryIMUversion = 2;
 
    
  }
  
  //Detect if BerryIMUv3 (Which uses the LSM6DSL and LIS3MDL) is connected
  readFrom(LSM6DSL_ADDRESS, LSM6DSL_WHO_AM_I,1,WHOresponse);
  LSM6DSL_WHO_AM_I_response = WHOresponse[0];
  
  readFrom(LIS3MDL_ADDRESS, LIS3MDL_WHO_AM_I,1,WHOresponse);
  LIS3MDL_WHO_AM_I_response = WHOresponse[0];

  if (LSM6DSL_WHO_AM_I_response == 0x6A && LIS3MDL_WHO_AM_I_response == 0x3D){
    Serial.println("\n\n   BerryIMUv3(LSM6DSL & LIS3MLD) found \n\n");
    BerryIMUversion = 3;
    
  }
  
  delay(1000);
  
}


void enableIMU(){
if (BerryIMUversion == 1){//For BerryIMUv1
  //Enable accelerometer
  writeTo(LSM9DS0_ACC_ADDRESS,LSM9DS0_CTRL_REG1_XM, 0b01100111); //  z,y,x axis enabled, continuos update,  100Hz data rate
  writeTo(LSM9DS0_ACC_ADDRESS,LSM9DS0_CTRL_REG2_XM, 0b00100000); // +/- 16G full scale

  //Enable the magnetometer
  writeTo(LSM9DS0_MAG_ADDRESS,LSM9DS0_CTRL_REG5_XM, 0b11110000);   // Temp enable, M data rate = 50Hz
  writeTo(LSM9DS0_MAG_ADDRESS,LSM9DS0_CTRL_REG6_XM, 0b01100000);   // +/-12gauss
  writeTo(LSM9DS0_MAG_ADDRESS,LSM9DS0_CTRL_REG7_XM, 0b00000000);   // Continuous-conversion mode

  // Enable Gyro
  writeTo(LSM9DS0_GYR_ADDRESS, LSM9DS0_CTRL_REG1_G, 0b00001111); // Normal power mode, all axes enabled
  writeTo(LSM9DS0_GYR_ADDRESS, LSM9DS0_CTRL_REG4_G, 0b00110000); // Continuos update, 2000 dps full scale
}
else if(BerryIMUversion == 2){//For BerryIMUv2  
      // Enable the gyroscope
    writeTo(LSM9DS1_GYR_ADDRESS,LSM9DS1_CTRL_REG4,0b00111000);      // z, y, x axis enabled for gyro
    writeTo(LSM9DS1_GYR_ADDRESS,LSM9DS1_CTRL_REG1_G,0b10111000);    // Gyro ODR = 476Hz, 2000 dps
    writeTo(LSM9DS1_GYR_ADDRESS,LSM9DS1_ORIENT_CFG_G,0b10111000);   // Swap orientation 

    // Enable the accelerometer
    writeTo(LSM9DS1_ACC_ADDRESS,LSM9DS1_CTRL_REG5_XL,0b00111000);   // z, y, x axis enabled for accelerometer
    writeTo(LSM9DS1_ACC_ADDRESS,LSM9DS1_CTRL_REG6_XL,0b00101000);   // +/- 16g

    //Enable the magnetometer
    writeTo(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG1_M, 0b10011100);   // Temp compensation enabled,Low power mode mode,80Hz ODR
    writeTo(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG2_M, 0b01000000);   // +/-12gauss
    writeTo(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG3_M, 0b00000000);   // continuos update
    writeTo(LSM9DS1_MAG_ADDRESS,LSM9DS1_CTRL_REG4_M, 0b00000000);   // lower power mode for Z axis
    
  }
else if(BerryIMUversion == 3){//For BerryIMUv3  

        //initialise the accelerometer
        writeTo(LSM6DSL_ADDRESS,LSM6DSL_CTRL1_XL,0b10011111);        // ODR 3.33 kHz, +/- 8g , BW = 400hz
        writeTo(LSM6DSL_ADDRESS,LSM6DSL_CTRL8_XL,0b11001000);        // Low pass filter enabled, BW9, composite filter
        writeTo(LSM6DSL_ADDRESS,LSM6DSL_CTRL3_C,0b01000100);         // Enable Block Data update, increment during multi byte read
        

        //initialise the gyroscope
        writeTo(LSM6DSL_ADDRESS,LSM6DSL_CTRL2_G,0b10011100);         // ODR 3.3 kHz, 2000 dps

        //initialise the magnetometer
        writeTo(LIS3MDL_ADDRESS,LIS3MDL_CTRL_REG1, 0b11011100);      // Temp sesnor enabled, High performance, ODR 80 Hz, FAST ODR disabled and Selft test disabled.
        writeTo(LIS3MDL_ADDRESS,LIS3MDL_CTRL_REG2, 0b00100000);      // +/- 8 gauss
        writeTo(LIS3MDL_ADDRESS,LIS3MDL_CTRL_REG3, 0b00000000);      // Continuous-conversion mode
 byte temp = 11;
    readFrom(LSM6DSL_ADDRESS,LIS3MDL_CTRL_REG3,1, temp);
    Serial.print(temp);
  }
}

void readACC(byte buff[]){
  if (BerryIMUversion == 1)
    readFrom(LSM9DS0_ACC_ADDRESS, 0x80 | LSM9DS0_OUT_X_L_A, 6, buff);
   else if(BerryIMUversion == 2 )//For BerryIMUv2  
    readFrom(LSM9DS1_ACC_ADDRESS, 0x80 | LSM9DS1_OUT_X_L_XL, 6, buff);
  else if(BerryIMUversion == 3 )//For BerryIMUv3  
    readFrom(LSM6DSL_ADDRESS, LSM6DSL_OUT_X_L_XL, 6, buff);
    

  
} 
 
void readMAG(byte buff[]){
  if (BerryIMUversion == 1)//For BerryIMUv1
    readFrom(LSM9DS0_MAG_ADDRESS, 0x80 | LSM9DS0_OUT_X_L_M, 6, buff);
  else if(BerryIMUversion == 2 )//For BerryIMUv2  
    readFrom(LSM9DS1_MAG_ADDRESS, 0x80 | LSM9DS1_OUT_X_L_M, 6, buff);
  else if(BerryIMUversion == 3 )//For BerryIMUv3  
    readFrom(LIS3MDL_ADDRESS, 0x80 | LIS3MDL_OUT_X_L, 6, buff);  
}

void readGYR(byte buff[]){
  if (BerryIMUversion == 1)//For BerryIMUv1
  readFrom(LSM9DS0_GYR_ADDRESS, 0x80 | LSM9DS0_OUT_X_L_G, 6, buff);
  else if(BerryIMUversion == 2 )//For BerryIMUv2  
  readFrom(LSM9DS1_GYR_ADDRESS, 0x80 | LSM9DS1_OUT_X_L_G, 6, buff);
  else if(BerryIMUversion == 3 )//For BerryIMUv3  
  readFrom(LSM6DSL_ADDRESS, LSM6DSL_OUT_X_L_G, 6, buff);
  
}
