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
int magRaw[3];
bool   running = true;
int magXmin = 32767;
int magYmin = 32767;
int magZmin = 32767;
int magXmax = -32767;
int magYmax = -32767;
int magZmax = -32767;


unsigned long startTime;

void setup() {

  Serial.begin(115200);  // start serial for output
  delay(500);
  detectIMU();
  
  enableIMU();


}

void loop() {
 
  if (running && Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'q' || c == 'Q') {
      running = false;
    Serial.println("Copy below calibration data into main program.");
    Serial.print("int magXmin = "); Serial.print(magXmin); Serial.println(";");
    Serial.print("int magYmin = "); Serial.print(magYmin); Serial.println(";");
    Serial.print("int magZmin = "); Serial.print(magZmin); Serial.println(";");
    Serial.print("int magXmax = "); Serial.print(magXmax); Serial.println(";");
    Serial.print("int magYmax = "); Serial.print(magYmax); Serial.println(";");
    Serial.print("int magZmax = "); Serial.print(magZmax); Serial.println(";");
    }
  }
  if (running){
    //Read the measurements from  compass
    readMAG(buff);
    magRaw[0] = (short)(buff[0] | (buff[1] << 8));   
    magRaw[1] = (short)(buff[2] | (buff[3] << 8));
    magRaw[2] = (short)(buff[4] | (buff[5] << 8));

    if(magRaw[0] > magXmax)
      magXmax = magRaw[0]; 
      if(magRaw[1] > magYmax)
      magYmax = magRaw[1]; 
      if(magRaw[2] > magZmax)
      magZmax = magRaw[2]; 

    if(magRaw[0] < magXmin)
      magXmin = magRaw[0];
      if(magRaw[1] < magYmin)
      magYmin = magRaw[1]; 
      if(magRaw[2] < magZmin)
      magZmin = magRaw[2]; 
                  
    Serial.print("###\tmagXmax\t");
    Serial.print(magXmax);
    Serial.print("\tmagYmax  ");
    Serial.print(magYmax );
    Serial.print("\tmagZmax");
    Serial.print(magZmax);
    Serial.print("\tmagXmin ");
    Serial.print(magXmin);
    Serial.print("\tmagYmin ");
    Serial.print(magYmin);
    Serial.print("\tmagZmin ");
    Serial.println(magZmin);
      
    while(millis() - startTime < DELAY_MS )
          {
              delay(5);
              /*Serial.println(millis() - startTime);
              Serial.print(LP*100);
              Serial.print(" ");
              Serial.println(LP*100);*/
          }
  }
 
}
