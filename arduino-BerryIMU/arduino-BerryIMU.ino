/*      This program  reads the angles and heading from the accelerometer, gyroscope
        and compass on a BerryIMU connected to an Arduino.


       The BerryIMUv1, BerryIMUv2, BerryIMUv3 and BerryIMU320G are supported


       Feel free to do whatever you like with this code.
       Distributed as-is; no warranty is given.

       https://ozzmaker.com/berryimu/


*/

#include "IMU.h"

#define DT  0.02          // Loop time
#define AA  0.50         // complementary filter constant
#define G_GAIN 0.070    // [deg/s/LSB]
#define DELAY_MS 10     // Delay in milliseconds added to loop

byte buff[6];
int accRaw[3];
int magRaw[3];
int gyrRaw[3];
float rate_gyr_y = 0.0;   // [deg/s]
float rate_gyr_x = 0.0;    // [deg/s]
float rate_gyr_z = 0.0;     // [deg/s]
float gyroXangle = 0.0;
float gyroYangle = 0.0;
float gyroZangle = 0.0;
float AccYangle = 0.0;
float AccXangle = 0.0;
float CFangleX = 0.0;
float CFangleY = 0.0;



/*################# Compass Calibration values ############
 Use calibrateBerryIMU.INO to get calibration values
 Calibrating the compass isnt mandatory, however a calibrated
 compass will result in a more accurate heading value.    
 ########################################################## */
int magXmin = 0;
int magYmin = 0;
int magZmin = 0;
int magXmax = 0;
int magYmax = 0;
int magZmax = 0;

/*int magXmin = -2294;
int magYmin = -366;
int magZmin = 2569;
int magXmax = -2020;
int magYmax = -296;
int magZmax = 2743;*/

unsigned long startTime;

void setup() {

  Serial.begin(115200);  // start serial for output
  delay(500);
  detectIMU();
  
  enableIMU();


}

void loop() {
 

  //Read the measurements from  sensors
  readACC(buff);
  accRaw[0] = (short)(buff[0] | (buff[1] << 8));   
  accRaw[1] = (short)(buff[2] | (buff[3] << 8));
  accRaw[2] = (short)(buff[4] | (buff[5] << 8));

  readMAG(buff);
  magRaw[0] = (short)(buff[0] | (buff[1] << 8));   
  magRaw[1] = (short)(buff[2] | (buff[3] << 8));
  magRaw[2] = (short)(buff[4] | (buff[5] << 8));


  readGYR(buff);
  gyrRaw[0] = (short)(buff[0] | (buff[1] << 8));   
  gyrRaw[1] = (short)(buff[2] | (buff[3] << 8));
  gyrRaw[2] = (short)(buff[4] | (buff[5] << 8));
  float LP = (millis() - startTime)/1000.0;
  startTime = millis();
  Serial.println (LP);
  
  
  //#Apply compass calibration
  magRaw[0] -= (magXmin + magXmax) /2;
  magRaw[1] -= (magYmin + magYmax) /2;
  magRaw[2] -= (magZmin + magZmax) /2;
  
  //Convert Gyro raw to degrees per second
    rate_gyr_x = (float) gyrRaw[0] * G_GAIN;
  rate_gyr_y = (float) gyrRaw[1]  * G_GAIN;
  rate_gyr_z = (float) gyrRaw[2]  * G_GAIN;

  
  
  //Calculate the angles from the gyro
  gyroXangle+=rate_gyr_x* LP;
  gyroYangle+=rate_gyr_y*LP;
  gyroZangle+=rate_gyr_z*LP;

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
  CFangleX=AA*(CFangleX+rate_gyr_x*LP) +(1 - AA) * AccXangle;
  CFangleY=AA*(CFangleY+rate_gyr_y*LP) +(1 - AA) * AccYangle;


  //Compute heading  
  float heading = 180 * atan2(magRaw[1],magRaw[0])/M_PI;
  
  //Convert heading to 0 - 360
          if(heading < 0)
            heading += 360;
            
  Serial.print("###\tAccX\t");
  Serial.print(AccXangle);
  Serial.print("\tAccY  ");
  Serial.print(AccYangle );
  
  Serial.print("\t###\tGyrX");
  Serial.print(gyroXangle);
  Serial.print("\tGyrY ");
  Serial.print(gyroYangle);
  Serial.print("\tGyrZ ");
  Serial.print(gyroZangle);
  Serial.print("\t######    CFangleX\t");
  Serial.print(CFangleX);
  Serial.print("   ######  CFangleY   \t");
  Serial.print(CFangleY);
  Serial.print("   ######  heading   \t");
  Serial.print(heading); 
  Serial.print("    --Loop Time--\t");

  
  while(millis() - startTime < DELAY_MS )
        {
            delay(5);
        }

}
