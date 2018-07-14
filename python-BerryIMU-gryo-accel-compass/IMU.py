import smbus
bus = smbus.SMBus(1)
from LSM9DS0 import *
from LSM9DS1 import *
import time



LSM9DS0 = 1



def detectIMU():
    #Detect which version of BerryIMU is connected.   
    #BerryIMUv1 uses the LSM9DS0
    #BerryIMUv2 uses the LSM9DS1
    global LSM9DS0
    
    
    try:
        #Check for LSM9DS0
        #If no LSM9DS0 is conencted, there will be an I2C bus error and the program will exit.
        #This section of code stops this from happening.
        LSM9DS0_WHO_G_response = (bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_WHO_AM_I_G))
        LSM9DS0_WHO_XM_response = (bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_WHO_AM_I_XM))
    except IOError as e:
        print ''        #need to do something here, so we just print a space
    else:
        if (LSM9DS0_WHO_G_response == 0xd4) and (LSM9DS0_WHO_XM_response == 0x49):
            print "Found LSM9DS0"
            LSM9DS0 = 1


    try:
        #Check for LSM9DS1
        #If no LSM9DS1 is conencted, there will be an I2C bus error and the program will exit.
        #This section of code stops this from happening.
        LSM9DS1_WHO_XG_response = (bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_WHO_AM_I_XG))
        LSM9DS1_WHO_M_response = (bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_WHO_AM_I_M))

    except IOError as f:
        print ''        #need to do something here, so we just print a space
    else:
        if (LSM9DS1_WHO_XG_response == 0x68) and (LSM9DS1_WHO_M_response == 0x3d):
            print "Found LSM9DS1"
            LSM9DS0 = 0

    time.sleep(1)



    


def writeAG(register,value):
        bus.write_byte_data(ACC_ADDRESS , register, value)
        return -1

def writeACC(register,value):
        bus.write_byte_data(ACC_ADDRESS , register, value)
        return -1

def writeMAG(register,value):
        bus.write_byte_data(MAG_ADDRESS, register, value)
        return -1

def writeGRY(register,value):
        bus.write_byte_data(GYR_ADDRESS, register, value)
        return -1



def readACCx():
        acc_l = bus.read_byte_data(ACC_ADDRESS, OUT_X_L_XL)
        acc_h = bus.read_byte_data(ACC_ADDRESS, OUT_X_H_XL)
	acc_combined = (acc_l | acc_h <<8)

	return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCy():
        acc_l = bus.read_byte_data(ACC_ADDRESS, OUT_Y_L_XL)
        acc_h = bus.read_byte_data(ACC_ADDRESS, OUT_Y_H_XL)
	acc_combined = (acc_l | acc_h <<8)

	return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCz():
        acc_l = bus.read_byte_data(ACC_ADDRESS, OUT_Z_L_XL)
        acc_h = bus.read_byte_data(ACC_ADDRESS, OUT_Z_H_XL)
	acc_combined = (acc_l | acc_h <<8)

	return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readMAGx():
		mag_l = bus.read_byte_data(MAG_ADDRESS, OUT_X_L_M)
		mag_h = bus.read_byte_data(MAG_ADDRESS, OUT_X_H_M)
		mag_combined = (mag_l | mag_h <<8)
		return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readMAGy():
        mag_l = bus.read_byte_data(MAG_ADDRESS, OUT_Y_L_M)
        mag_h = bus.read_byte_data(MAG_ADDRESS, OUT_Y_H_M)
        mag_combined = (mag_l | mag_h <<8)

        return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readMAGz():
        mag_l = bus.read_byte_data(MAG_ADDRESS, OUT_Z_L_M)
        mag_h = bus.read_byte_data(MAG_ADDRESS, OUT_Z_H_M)
        mag_combined = (mag_l | mag_h <<8)

        return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readGYRx():
        gyr_l = bus.read_byte_data(GYR_ADDRESS, OUT_X_L_G)
        gyr_h = bus.read_byte_data(GYR_ADDRESS, OUT_X_H_G)
        gyr_combined = (gyr_l | gyr_h <<8)

        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536
  

def readGYRy():
        gyr_l = bus.read_byte_data(GYR_ADDRESS, OUT_Y_L_G)
        gyr_h = bus.read_byte_data(GYR_ADDRESS, OUT_Y_H_G)
        gyr_combined = (gyr_l | gyr_h <<8)

        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
        gyr_l = bus.read_byte_data(GYR_ADDRESS, OUT_Z_L_G)
        gyr_h = bus.read_byte_data(GYR_ADDRESS, OUT_Z_H_G)
        gyr_combined = (gyr_l | gyr_h <<8)

        return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536









def writeACC(register,value):
    if(LSM9DS0):
        bus.write_byte_data(LSM9DS0_ACC_ADDRESS , register, value)
    else:
        bus.write_byte_data(LSM9DS1_ACC_ADDRESS , register, value)
    return -1
    

def writeMAG(register,value):
    if(LSM9DS0):
        bus.write_byte_data(LSM9DS0_MAG_ADDRESS, register, value)
    else:
        bus.write_byte_data(LSM9DS1_MAG_ADDRESS , register, value)        
        return -1

def writeGRY(register,value):
    if(LSM9DS0):
        bus.write_byte_data(LSM9DS0_GYR_ADDRESS, register, value)
    else:
        bus.write_byte_data(LSM9DS1_GYR_ADDRESS , register, value)        
        return -1



def readACCx():
    if (LSM9DS0):
        acc_l = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_X_L_A)
        acc_h = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_X_H_A)
    else:
        acc_l = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_X_L_XL)
        acc_h = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_X_H_XL)

    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCy():
    if (LSM9DS0):
        acc_l = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Y_L_A)
        acc_h = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Y_H_A)
    else:
        acc_l = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Y_L_XL)
        acc_h = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Y_H_XL)       
    
    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readACCz():
    if (LSM9DS0):
        acc_l = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Z_L_A)
        acc_h = bus.read_byte_data(LSM9DS0_ACC_ADDRESS, LSM9DS0_OUT_Z_H_A)
    else:
        acc_l = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Z_L_XL)
        acc_h = bus.read_byte_data(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_Z_H_XL)
	
    acc_combined = (acc_l | acc_h <<8)
    return acc_combined  if acc_combined < 32768 else acc_combined - 65536


def readMAGx():
    if (LSM9DS0):
        mag_l = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_X_L_M)
        mag_h = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_X_H_M)
    else:
        mag_l = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_X_L_M)
        mag_h = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_X_H_M)

    mag_combined = (mag_l | mag_h <<8)
    return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readMAGy():
    if (LSM9DS0):
        mag_l = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Y_L_M)
        mag_h = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Y_H_M)
    else:
        mag_l = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Y_L_M)
        mag_h = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Y_H_M)

    mag_combined = (mag_l | mag_h <<8)
    return mag_combined  if mag_combined < 32768 else mag_combined - 65536


def readMAGz():
    if (LSM9DS0):
        mag_l = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Z_L_M)
        mag_h = bus.read_byte_data(LSM9DS0_MAG_ADDRESS, LSM9DS0_OUT_Z_H_M)
    else:
        mag_l = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Z_L_M)
        mag_h = bus.read_byte_data(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_Z_H_M)           

    mag_combined = (mag_l | mag_h <<8)
    return mag_combined  if mag_combined < 32768 else mag_combined - 65536



def readGYRx():
    if (LSM9DS0):
        gyr_l = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_X_L_G)
        gyr_h = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_X_H_G)
    else:
        gyr_l = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_X_L_G)
        gyr_h = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_X_H_G)
    
    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536
  

def readGYRy():
    if (LSM9DS0):
        gyr_l = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Y_L_G)
        gyr_h = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Y_H_G)
    else:
        gyr_l = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Y_L_G)
        gyr_h = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Y_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536

def readGYRz():
    if (LSM9DS0):
        gyr_l = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Z_L_G)
        gyr_h = bus.read_byte_data(LSM9DS0_GYR_ADDRESS, LSM9DS0_OUT_Z_H_G)
    else:
        gyr_l = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Z_L_G)
        gyr_h = bus.read_byte_data(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_Z_H_G)

    gyr_combined = (gyr_l | gyr_h <<8)
    return gyr_combined  if gyr_combined < 32768 else gyr_combined - 65536




def initIMU():

    if (LSM9DS0):   #For BerryIMUv1

        #initialise the accelerometer
        writeACC(LSM9DS0_CTRL_REG1_XM, 0b01100111)  #z,y,x axis enabled, continuos update,  100Hz data rate
        writeACC(LSM9DS0_CTRL_REG2_XM, 0b00100000)  #+/- 16G full scale

        #initialise the magnetometer
        writeMAG(LSM9DS0_CTRL_REG5_XM, 0b11110000)  #Temp enable, M data rate = 50Hz
        writeMAG(LSM9DS0_CTRL_REG6_XM, 0b01100000)  #+/-12gauss
        writeMAG(LSM9DS0_CTRL_REG7_XM, 0b00000000)  #Continuous-conversion mode

        #initialise the gyroscope
        writeGRY(LSM9DS0_CTRL_REG1_G, 0b00001111)   #Normal power mode, all axes enabled
        writeGRY(LSM9DS0_CTRL_REG4_G, 0b00110000)   #Continuos update, 2000 dps full scale

    else:       #For BerryIMUv2
        #initialise the gyroscope
        writeGRY(LSM9DS1_CTRL_REG4,0b00111000)      #z, y, x axis enabled for gyro
        writeGRY(LSM9DS1_CTRL_REG1_G,0b10111000)    #Gyro ODR = 476Hz, 2000 dps
        writeGRY(LSM9DS1_ORIENT_CFG_G,0b00111000)   #Swap orientation 

        #initialise the accelerometer
        writeACC(LSM9DS1_CTRL_REG5_XL,0b00111000)   #z, y, x axis enabled for accelerometer
        writeACC(LSM9DS1_CTRL_REG6_XL,0b00101000)   #+/- 16g

        #initialise the magnetometer
        writeMAG(LSM9DS1_CTRL_REG1_M, 0b10011100)   #Temp compensation enabled,Low power mode mode,80Hz ODR
        writeMAG(LSM9DS1_CTRL_REG2_M, 0b01000000)   #+/-12gauss
        writeMAG(LSM9DS1_CTRL_REG3_M, 0b00000000)   #continuos update
        writeMAG(LSM9DS1_CTRL_REG4_M, 0b00000000)   #lower power mode for Z axis













