/*
  This program  reads the angles and heading from the accelerometer, gyroscope
   and compass on a BerryIMU connected to an Arduino.
  

    Both the BerryIMUv1 and BerryIMUv2 are supported.
    Feel free to do whatever you like with this code
    Distributed as-is; no warranty is given.
    http://ozzmaker.com/
*/


#include <Arduino.h>
#include <Wire.h>
#include "LSM9DS0.h"
#include "LSM9DS1.h"


int LSM9DS0 = 0;
int LSM9DS1 = 0;



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
  Wire.begin(); 
  byte LSM9DS0_WHO_AM_I_G_response;
  byte LSM9DS0_WHO_AM_I_XM_response;
  byte LSM9DS1_WHO_M_response;
  byte LSM9DS1_WHO_XG_response;

  
  byte WHOresponse[2];

  //Detect if BerryIMUv1 (Which uses a LSM9DS0) is connected
  readFrom(LSM9DS0_GYR_ADDRESS, LSM9DS0_WHO_AM_I_G,1,WHOresponse);
  LSM9DS0_WHO_AM_I_G_response = WHOresponse[0];
  
  readFrom(LSM9DS0_ACC_ADDRESS, LSM9DS0_WHO_AM_I_XM,1,WHOresponse);
  LSM9DS0_WHO_AM_I_XM_response = WHOresponse[0];

  if (LSM9DS0_WHO_AM_I_G_response == 0xD4 && LSM9DS0_WHO_AM_I_XM_response == 0x49){
    Serial.println("\n\n   LSM9DS0 found \n\n");
    LSM9DS0 = 1;
  }

  
  //Detect if BerryIMUv2 (Which uses a LSM9DS1) is connected
  readFrom(LSM9DS1_MAG_ADDRESS, LSM9DS1_WHO_AM_I_M,1,WHOresponse);
  LSM9DS1_WHO_M_response = WHOresponse[0];
  
  readFrom(LSM9DS1_GYR_ADDRESS, LSM9DS1_WHO_AM_I_XG,1,WHOresponse);
  LSM9DS1_WHO_XG_response = WHOresponse[0];

  if (LSM9DS1_WHO_XG_response == 0x68 && LSM9DS1_WHO_M_response == 0x3D){
    Serial.println("\n\n   LSM9DS1 found \n\n");
    LSM9DS1 = 1;
  }
  
    
  delay(1000);
  
  
  
  


  }


void enableIMU(){


if (LSM9DS0){//For BerryIMUv1
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
else if(LSM9DS1){//For BerryIMUv2  
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
}

void readACC(byte buff[]){
  if (LSM9DS0)//For BerryIMUv1
    readFrom(LSM9DS0_ACC_ADDRESS, 0x80 | LSM9DS0_OUT_X_L_A, 6, buff);
  
  else if(LSM9DS1)//For BerryIMUv2  
    readFrom(LSM9DS1_ACC_ADDRESS, 0x80 | LSM9DS1_OUT_X_L_XL, 6, buff);
} 
 
void readMAG(byte buff[]){
  if (LSM9DS0)//For BerryIMUv1
    readFrom(LSM9DS0_MAG_ADDRESS, 0x80 | LSM9DS0_OUT_X_L_M, 6, buff);
  else if(LSM9DS1)//For BerryIMUv2  
    readFrom(LSM9DS1_MAG_ADDRESS, 0x80 | LSM9DS1_OUT_X_L_M, 6, buff);
  
}

void readGYR(byte buff[]){
  if (LSM9DS0)//For BerryIMUv1
  readFrom(LSM9DS0_GYR_ADDRESS, 0x80 | LSM9DS0_OUT_X_L_G, 6, buff);
  else if(LSM9DS1)//For BerryIMUv2 
  readFrom(LSM9DS1_GYR_ADDRESS, 0x80 | LSM9DS1_OUT_X_L_G, 6, buff);
}


