#include "bmp180.h"
#include <Arduino.h> 
#include <Wire.h>

char _error;
byte buff[6];
double c5,c6,mc,md,x0,x1,x2,yy0,yy1,y2,p0,p1,p2;
int16_t AC1,AC2,AC3,VB1,VB2,MB,MC,MD;
uint16_t AC4,AC5,AC6; 


// Write to I2C
char writeTo(int device, byte address, byte val) {
  Wire.beginTransmission(device);  //start transmission to device 
  Wire.write(address);             //send register address
  Wire.write(val);                 //send value to write
  _error = Wire.endTransmission(); //end transmission
  if (_error == 0)
    return(1);
  else
    return(0);
}

// Read from I2C
char readFrom(int device, byte address, int num, byte buff[]) {
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
  _error = Wire.endTransmission(); //end transmission
  if (_error == 0)
    return(1);
  else
    return(0);
}



char startPressure(char oversampling)
// Begin a pressure reading.
// Oversampling: 0 to 3, higher numbers are slower, higher-res outputs.
// Will return delay in ms to wait, or 0 if I2C error.

{
  unsigned char data[2], result, delay;
  
  data[0] = BMP180_REG_CONTROL;

  switch (oversampling)
  {
    case 0:
      data[1] = BMP180_COMMAND_PRESSURE0;
      delay = 5;
    break;
    case 1:
      data[1] = BMP180_COMMAND_PRESSURE1;
      delay = 8;
    break;
    case 2:
      data[1] = BMP180_COMMAND_PRESSURE2;
      delay = 14;
    break;
    case 3:
      data[1] = BMP180_COMMAND_PRESSURE3;
      delay = 26;
    break;
    default:
      data[1] = BMP180_COMMAND_PRESSURE0;
      delay = 5;
    break;
  }

result = writeTo(BMP180_ADDR,BMP180_REG_CONTROL,data[1]);
  if (result) // good write?
    return(delay); // return the delay in ms (rounded up) to wait before retrieving data
  else
    return(0); // or return 0 if there was a problem communicating with the BMP
}










double sealevel(double P, double A)
// Given a pressure P (mb) taken at a specific altitude (meters),
// return the equivalent pressure (mb) at sea level.
// This produces pressure readings that can be used for weather measurements.
{
  return(P/pow(1-(A/44330.0),5.255));
}


double altitude(double P, double P0)
// Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
// return altitude (meters) above baseline.
{
  return(44330.0*(1-pow(P/P0,1/5.255)));
   
}




char getPressure(double &P, double &T)
// Retrieve a previously started pressure reading, calculate absolute pressure in mbars.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startPressure() to have been called prior and sufficient time elapsed.
// Requires recent temperature reading to accurately calculate pressure.

// P: external variable to hold pressure.
// T: previously-calculated temperature.
// Returns 1 for success, 0 for I2C error.

// Note that calculated pressure value is absolute mbars, to compensate for altitude call sealevel().
{
  unsigned char data[3];
  char result;
  double pu,s,x,y,z;
  result = 1;
  data[0] = BMP180_REG_RESULT;

  //result = readBytes(data, 3);
  readFrom(BMP180_ADDR,BMP180_REG_RESULT,2,data);
  
  if (result) // good read, calculate pressure
  {
    pu = (data[0] * 256.0) + data[1] + (data[2]/256.0);

    //example from Bosch datasheet
    //pu = 23843;

    //example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf, pu = 0x982FC0; 
    //pu = (0x98 * 256.0) + 0x2F + (0xC0/256.0);
    
    s = T - 25.0;
    x = (x2 * pow(s,2)) + (x1 * s) + x0;
    y = (y2 * pow(s,2)) + (yy1 * s) + yy0;
    z = (pu - x) / y;
    P = (p2 * pow(z,2)) + (p1 * z) + p0;

  }
  return(result);
}










void readCalBMP180(){
double c3,c4,b1;



  //Signed two'scomplement calibration values
  readFrom(BMP180_ADDR,0xAA,2,buff);
  AC1 = (int16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xAC,2,buff);
  AC2 = (int16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xAE,2,buff);
  AC3 = (int16_t)((buff[0]<<8)|buff[1]);


  //Unsigned two'scomplement calibration values
  readFrom(BMP180_ADDR,0xB0,2,buff);
  AC4 = (uint16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xB2,2,buff);
  AC5 = (uint16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xB4,2,buff);
  AC6 = (uint16_t)((buff[0]<<8)|buff[1]);
  
  //Signed two'scomplement calibration values
  readFrom(BMP180_ADDR,0xB6,2,buff);
  VB1 = (int16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xB8,2,buff);
  VB2 = (int16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xBA,2,buff);
  MB = (int16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xBC,2,buff);
  MC = (int16_t)((buff[0]<<8)|buff[1]);
  
  readFrom(BMP180_ADDR,0xBE,2,buff);
  MD = (int16_t)((buff[0]<<8)|buff[1]);
  
    // Compute floating-point polynominals:

  c3 = 160.0 * pow(2,-15) * AC3;
  c4 = pow(10,-3) * pow(2,-15) * AC4;
  b1 = pow(160,2) * pow(2,-30) * VB1;
  c5 = (pow(2,-15) / 160) * AC5;
  c6 = AC6;
  mc = (pow(2,11) / pow(160,2)) * MC;
  md = MD / 160.0;
  x0 = AC1;
  x1 = 160.0 * pow(2,-13) * AC2;
  x2 = pow(160,2) * pow(2,-25) * VB2;
  yy0 = c4 * pow(2,15);
  yy1 = c4 * c3;
  y2 = c4 * b1;
  p0 = (3791.0 - 8.0) / 1600.0;
  p1 = 1.0 - 7357.0 * pow(2,-20);
  p2 = 3038.0 * 100.0 * pow(2,-36);
}

 

char getTemperature(double &T)
// Retrieve a previously-started temperature reading.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startTemperature() to have been called prior and sufficient time elapsed.
// T: external variable to hold result.
// Returns 1 if successful, 0 if I2C error.
{
  unsigned char data[2];
  char result;
  double tu, a;
  
  data[0] = BMP180_REG_RESULT;
 
  result = readFrom(BMP180_ADDR,BMP180_REG_RESULT,2,data);
  if (result) // good read, calculate temperature
  {
    tu = (data[0] * 256.0) + data[1];

    //example from Bosch datasheet
    //tu = 27898;

    //example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf
    //tu = 0x69EC;
    
    a = c5 * (tu - c6);
    T = a + (mc / (a + md));

    /*    
    Serial.println();
    Serial.print("tu: "); Serial.println(tu);
    Serial.print("a: "); Serial.println(a);
    Serial.print("T: "); Serial.println(*T);
    */
  }
  return(result);
}

char startTemperature(void)
// Begin a temperature reading.
// Will return delay in ms to wait, or 0 if I2C error
{
  unsigned char data[2], result;
  
  data[0] = BMP180_REG_CONTROL;
  data[1] = BMP180_COMMAND_TEMPERATURE;
  result = writeTo (BMP180_ADDR,BMP180_REG_CONTROL,data[1]);
  if (result) // good write?
    return(5); // return the delay in ms (rounded up) to wait before retrieving data
  else
    return(0); // or return 0 if there was a problem communicating with the BMP
}


