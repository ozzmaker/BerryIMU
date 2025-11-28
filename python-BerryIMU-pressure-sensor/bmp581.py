import time
from datetime import datetime, timedelta
import smbus
import RPi.GPIO as GPIO


BMP581_ADDRESS = 0x47  # SDO is low 0x46

BMP581_SDO_PIN = 33
BMP581_INT_PIN = 35

GPIO.setmode(GPIO.BOARD)
i2c = smbus.SMBus(1)

def BH1750FVI_getdata():
    data = i2c.read_i2c_block_data(BH1750FVI_ADDRESS, 0x10,2)
    AL = (data[0] << 8 | data[1])/ 1.2
    return AL

def BMP581_init():
    GPIO.setup(BMP581_SDO_PIN, GPIO.OUT, initial = GPIO.LOW)
    GPIO.setup(BMP581_INT_PIN, GPIO.IN)
    time.sleep(0.1)
    GPIO.output(BMP581_SDO_PIN, GPIO.LOW)
    time.sleep(0.1)

    if(i2c.read_byte_data(BMP581_ADDRESS,0x01) == 0x00):
        raise ValueError("BMP581 CHIP_ID Check Error")
    if(i2c.read_byte_data(BMP581_ADDRESS, 0x28) & 0x02 != 0x02):
        raise ValueError("BMP581 STATUS_NVM_RDY Check Error")
    if(i2c.read_byte_data(BMP581_ADDRESS, 0x27) != 0x10):
        raise ValueError("BMP581 INT Check Error")

    i2c.write_byte_data(BMP581_ADDRESS, 0x36, 0x7F)
    i2c.write_byte_data(BMP581_ADDRESS, 0x31, 0x09)
    i2c.write_byte_data(BMP581_ADDRESS, 0x37, 0xEB)

def BMP581_getdata():
    data = i2c.read_i2c_block_data(BMP581_ADDRESS, 0x1D, 6)

    temp = (data[2] << 16 | data[1] << 8 | data[0]) / (1 << 16)
    pressure = (data[5] << 16 | data[4] << 8 | data[3]) / (1 << 6)

    return temp, pressure



if __name__ == '__main__':
    try:
        BMP581_init()
    except OSError as e:
            print("BMP581 Init Error: ", e)
    except ValueError as e:
            print(e)
    while True:
        #BMP581
        try:
            BMP581_Temperature, BMP581_Pressure = -2.0, -2.0
            BMP581_Temperature, BMP581_Pressure = BMP581_getdata()
            print(
                 "BMP581_Temperature: {:.2f} C    BMP581_Pressure: {:.2f} Pa ".format(
                 BMP581_Temperature, BMP581_Pressure
                     )
              )
        except OSError as e:
            print("BMP581 I2C Communication Error: ", e)
        except ValueError as e:
            print(e)

        time.sleep(0.5)
        print(BMP581_Pressure) 
        print(BMP581_Temperature) 
    GPIO.cleanup()
    print("GPIO cleaned up and program exited.")

