/*
  This program  reads the angles and heading from the accelerometer, gyroscope
   and compass on a BerryIMU connected to an ESP8266. It also creates a simple
   web page which shows these values and refreshes every 1 seconds.
   
  http://ozzmaker.com/
  
    Copyright (C) 2016  Mark Williams
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




#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "LSM9DS0.h"


const char* ssid = "******";
const char* password = "***************";




#define DT  0.02          // Loop time
#define AA  0.97          // complementary filter constant
#define G_GAIN 0.070      // [deg/s/LSB]

int xGyro, yGyro, zGyro, xAccl, yAccl, zAccl, xMag, yMag, zMag;
byte buff[6];
short accRaw[3];
short magRaw[3];
short gyrRaw[3];
float rate_gyr_y = 0.0;     // [deg/s]
float rate_gyr_x = 0.0;     // [deg/s]
float rate_gyr_z = 0.0;     // [deg/s]
float gyroXangle = 0.0;
float gyroYangle = 0.0;
float gyroZangle = 0.0;
float AccYangle = 0.0;
float AccXangle = 0.0;
float CFangleX = 0.0;
float CFangleY = 0.0;
float accXnorm,accYnorm,pitch,roll,magXcomp,magYcomp,heading,headingComp;
unsigned long startTime;

ESP8266WebServer server(80);

void handleroot()
{

  //Create webpage with BerryIMU data which is updated every 1 seconds
  server.sendContent("HTTP/1.1 200 OK\r\n"); //send new page
  server.sendContent("Content-Type: text/html\r\n");
  server.sendContent("\r\n");
  server.sendContent
  ("<html><head><meta http-equiv='refresh' content='1'</meta>"
  "<h3 style=text-align:center;font-size:200%;color:RED;>BerryIMU and ESP8266</h3>"
  "<h3 style=text-align:center;font-size:100%;>accelerometer, gyroscope, magnetometer</h3>"
  "<h3 style=text-align:center;font-family:courier new;><a href=http://ozzmaker.com/ target=_blank>http://ozzmaker.com</a></h3><hr>");
  server.sendContent
  ("<h2 style=text-align:center;> Filtered X angle= " + String(CFangleX));
  server.sendContent
  ("<h2 style=text-align:center;> Filtered Y angle= " + String(CFangleY));
  server.sendContent
  ("</h2><h2 style=text-align:center;> Heading = " + String(heading));
  server.sendContent
  ("</h2><h2 style=text-align:center;> Tilt compensated heading =  " + String(headingComp));
}

void setup()
{
  // Initialise I2C communication
   Wire.begin(4,5);  //Wire.begin([SDA], [SCL])
  // Initialise serial communication, set baud rate = 115200
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  //Print IP to console
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  delay(3);
  
  // Start the server
  server.on("/", handleroot);
  server.begin();
  Serial.println("HTTP server started");

  // Enable accelerometer
  writeTo(ACC_ADDRESS,CTRL_REG1_XM, 0b01100111); //  z,y,x axis enabled, continuos update,  100Hz data rate
  writeTo(ACC_ADDRESS,CTRL_REG2_XM, 0b00100000); // +/- 16G full scale

  //Enable the magnetometer
  writeTo(MAG_ADDRESS,CTRL_REG5_XM, 0b11110000);   // Temp enable, M data rate = 50Hz
  writeTo(MAG_ADDRESS,CTRL_REG6_XM, 0b01100000);   // +/-12gauss
  writeTo(MAG_ADDRESS,CTRL_REG7_XM, 0b00000000);   // Continuous-conversion mode

  // Enable Gyro
  writeTo(GYR_ADDRESS, CTRL_REG1_G, 0b00001111); // Normal power mode, all axes enabled
  writeTo(GYR_ADDRESS, CTRL_REG4_G, 0b00110000); // Continuos update, 2000 dps full scale
}



void loop()
{
  server.handleClient();  //Handler for client connections
  startTime = millis();   //Used to calculate loop time. Needed for gyro calculations

  //Read the measurements from the sensors
  readFrom(ACC_ADDRESS, 0x80 | OUT_X_L_A, 6, buff);
  accRaw[0] = (int)(buff[0] | (buff[1] << 8));   
  accRaw[1] = (int)(buff[2] | (buff[3] << 8));
  accRaw[2] = (int)(buff[4] | (buff[5] << 8));
  readFrom(MAG_ADDRESS, 0x80 | OUT_X_L_M, 6, buff);
  magRaw[0] = (int)(buff[0] | (buff[1] << 8));   
  magRaw[1] = (int)(buff[2] | (buff[3] << 8));
  magRaw[2] = (int)(buff[4] | (buff[5] << 8));
  readFrom(GYR_ADDRESS, 0x80 | OUT_X_L_G, 6, buff);
  gyrRaw[0] = (int)(buff[0] | (buff[1] << 8));   
  gyrRaw[1] = (int)(buff[2] | (buff[3] << 8));
  gyrRaw[2] = (int)(buff[4] | (buff[5] << 8));
  
  //Convert Gyro raw to degrees per second
  rate_gyr_x = (float) gyrRaw[0] * G_GAIN;
  rate_gyr_y = (float) gyrRaw[1] * G_GAIN;
  rate_gyr_z = (float) gyrRaw[2] * G_GAIN;

  //Calculate the angles from the gyro
  gyroXangle+=rate_gyr_x*DT;
  gyroYangle+=rate_gyr_y*DT;
  gyroZangle+=rate_gyr_z*DT;

  //Convert Accelerometer values to degrees
  AccXangle = (float) (atan2(accRaw[1],accRaw[2])+M_PI)*RAD_TO_DEG;
  AccYangle = (float) (atan2(accRaw[2],accRaw[0])+M_PI)*RAD_TO_DEG;


  //If IMU is up the correct way, use these lines
  AccXangle -= (float)180.0;
  if (AccYangle > 90)
    AccYangle -= (float)270;
  else
    AccYangle += (float)90;


  //Complementary filter used to combine the accelerometer and gyro values.
  CFangleX=AA*(CFangleX+rate_gyr_x*DT) +(1 - AA) * AccXangle;
  CFangleY=AA*(CFangleY+rate_gyr_y*DT) +(1 - AA) * AccYangle;


  //Compute heading  
  heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;
  
  //Convert heading to 0 - 360
  if(heading < 0)
    heading += 360;

  //Normalize accelerometer raw values. This is used for tilt compensated heading
  accXnorm = accRaw[0]/sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);
  accYnorm = accRaw[1]/sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);

  //Calculate pitch and roll
  pitch = asin(accXnorm);
  roll = -asin(accYnorm/cos(pitch));

  //Calculate the new tilt compensated values
  magXcomp = magRaw[0]*cos(pitch)+magRaw[02]*sin(pitch);
  magYcomp = magRaw[0]*sin(roll)*sin(pitch)+magRaw[1]*cos(roll)-magRaw[2]*sin(roll)*cos(pitch);


  //Calculate heading
  headingComp = 180*atan2(magYcomp,magXcomp)/M_PI;

  //Convert heading to 0 - 360
  if(headingComp < 0)
    headingComp += 360;

          
  Serial.print("AccX ");
  Serial.print(AccXangle);
  Serial.print("\tAccY ");
  Serial.print(AccYangle);
  Serial.print("\t### GyrX ");
  Serial.print(gyroXangle);
  Serial.print("\tGyrY ");
  Serial.print(gyroYangle);
  Serial.print("\tGyrZ ");
  Serial.print(gyroZangle);
  Serial.print("\t### CFangleX ");
  Serial.print(CFangleX);
  Serial.print("\tCFangleY ");
  Serial.print(CFangleY);
  Serial.print("\t###  heading ");
  Serial.print(heading); 
  Serial.print("\tCompensated  Heading ");
  Serial.print(headingComp);
  Serial.print("  --Loop Time-- ");



    
  //Each loop should be at least DT seconds. This is needed to calculate gyro angles
  while(millis() - startTime < (DT*1000)){
            delay(1);
   }
  Serial.print( millis()- startTime);
  Serial.println(" ");

  
  
}

void writeTo(int device, byte address, byte val) {
   Wire.beginTransmission(device);  //start transmission to device 
   Wire.write(address);             //send register address
   Wire.write(val);                 //send value to write
   Wire.endTransmission();          //end transmission
}

void readFrom(int device, byte address, int num, byte buff[]) {
  Wire.beginTransmission(device);   //start transmission to device 
  Wire.write(address);              //sends address to read from
  Wire.endTransmission();           //end transmission
  
  Wire.beginTransmission(device);   //start transmission to device (initiate again)
  Wire.requestFrom(device, num);    //request 6 bytes from device
  
  int i = 0;
  while(Wire.available())           //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read();          //receive a byte
    i++;
  }
  Wire.endTransmission();           //end transmission
}



