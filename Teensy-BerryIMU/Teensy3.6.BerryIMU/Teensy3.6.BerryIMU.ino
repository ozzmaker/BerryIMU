/*
  This program  reads the angles and heading from the accelerometer, gyroscope
   and compass on a BerryIMU connected to a Teensy 3.6.
  
  http://ozzmaker.com/

*/

#include <i2c_t3.h>
#include "LSM9DS0.h"

#define DT  0.02          // Loop time E.g 0.02 = 20 milliseconds
#define AA  0.80         // complementary filter constant
#define G_GAIN 0.070    // [deg/s/LSB]

byte buff[6];
int accRaw[3];
int magRaw[3];
int gyrRaw[3];
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



unsigned long startTime;

void setup() {
  Wire.begin();        // Initialise i2c 
  Wire.setClock(400000);  //Change i2c bus speed to 400kHz
  Serial.begin(115200);  // start serial for output
  
  //Enable accelerometer
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

void loop() {

 startTime = millis();

 
    
  //Read the measurements from the sensors, combine and convert to correct values
  //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value) 
  //Start at OUT_X_L_A and read 6 bytes.
  readFrom(ACC_ADDRESS, 0x80 | OUT_X_L_A, 6, buff);
  accRaw[0] = (int)(buff[0] | (buff[1] << 8));   
  accRaw[1] = (int)(buff[2] | (buff[3] << 8));
  accRaw[2] = (int)(buff[4] | (buff[5] << 8));
  if (accRaw[0] >= 32768) accRaw[0] = accRaw[0] - 65536;
  if (accRaw[1] >= 32768) accRaw[1] = accRaw[1] - 65536;
  if (accRaw[2] >= 32768) accRaw[2] = accRaw[2] - 65536;
  
  readFrom(MAG_ADDRESS, 0x80 | OUT_X_L_M, 6, buff);
  magRaw[0] = (int)(buff[0] | (buff[1] << 8));   
  magRaw[1] = (int)(buff[2] | (buff[3] << 8));
  magRaw[2] = (int)(buff[4] | (buff[5] << 8));
  if (magRaw[0] >= 32768) magRaw[0] = magRaw[0]- 65536;
  if (magRaw[1] >= 32768) magRaw[1] = magRaw[1]- 65536;
  if (magRaw[2] >= 32768) magRaw[2] = magRaw[2]- 65536;

  readFrom(GYR_ADDRESS, 0x80 | OUT_X_L_G, 6, buff);
  gyrRaw[0] = (int)(buff[0] | (buff[1] << 8));   
  gyrRaw[1] = (int)(buff[2] | (buff[3] << 8));
  gyrRaw[2] = (int)(buff[4] | (buff[5] << 8));
  if (gyrRaw[0] >= 32768) gyrRaw[0] = gyrRaw[0]- 65536;
  if (gyrRaw[1] >= 32768) gyrRaw[1] = gyrRaw[1]- 65536;
  if (gyrRaw[2] >= 32768) gyrRaw[2] = gyrRaw[2]- 65536;
  

   
  //Convert Gyro raw to degrees per second
  rate_gyr_x = (float) gyrRaw[0] * G_GAIN;
  rate_gyr_y = (float) gyrRaw[1]  * G_GAIN;
  rate_gyr_z = (float) gyrRaw[2]  * G_GAIN;

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
  float heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;
  
  //Convert heading to 0 - 360
          if(heading < 0)
            heading += 360;

  
           
  Serial.print("AccX\t");
  Serial.print(AccXangle);
  Serial.print("\t#  AccY\t");
  Serial.print(AccYangle);
  Serial.print("\t#  GyrX\t");
  Serial.print(gyroXangle);
  Serial.print("\t#  GyrY\t");
  Serial.print(gyroYangle);
  Serial.print("\t#  GyrZ\t");
  Serial.print(gyroZangle);
  Serial.print("\t# CFangleX\t");
  Serial.print(CFangleX);
  Serial.print("\t# CFangleY\t");
  Serial.print(CFangleY);
  Serial.print("\t# heading\t");
  Serial.print(heading); 
  Serial.print("    --Loop Time--\t");



  //Each loop should be at least DT
  while(millis() - startTime < (DT*1000))
        {
            delay(1);
        }
  Serial.print( millis()- startTime);
  Serial.println();

}




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


