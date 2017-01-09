
#include <stdint.h>


#ifndef _DEF_BMP180_
#define _DEF_BMP180_


#define ALTITUDE 89.7 // Altitude OzzMaker


#define BMP180_ADDR 0x77 // 7-bit address

#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6

#define BMP180_COMMAND_TEMPERATURE 0x2E
#define BMP180_COMMAND_PRESSURE0 0x34
#define BMP180_COMMAND_PRESSURE1 0x74
#define BMP180_COMMAND_PRESSURE2 0xB4
#define BMP180_COMMAND_PRESSURE3 0xF4



void readCalBMP180();
char getTemperature(double &T);
char startTemperature(void);
char startPressure(char oversampling);
char getPressure(double &P, double &T);
void readCalBMP180();
double sealevel(double P, double A);
double altitude(double P, double P0);




#endif
























