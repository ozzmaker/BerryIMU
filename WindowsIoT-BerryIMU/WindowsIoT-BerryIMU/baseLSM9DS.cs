using System;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace BerryImu
{
    struct Gyroscope
    {
        public double rawX;
        public double rawY;
        public double rawZ;
    };
    struct Accelerometer
    {
        public double rawX;
        public double rawY;
        public double rawZ;
    };
    struct Magnetometer
    {
        public double rawX;
        public double rawY;
        public double rawZ;
        public double headingInDegrees;
        public double headingInDegreesTiltCompensated;
    };

    internal abstract class baseLSM9DS : IDisposable
    {
        protected I2cDevice i2cDeviceGyroscope;
        protected I2cDevice i2cDeviceAccelerometer;
        protected I2cDevice i2cDeviceMagnetometer;

        //I2C addresses
        public abstract byte MAG_ADDRESS { get; }
        public abstract byte ACC_ADDRESS { get; }
        public abstract byte GYR_ADDRESS { get; }
        public abstract byte GYR_OUT_X_L_G { get; }
        public abstract byte ACC_OUT_X_L { get; }
        public abstract byte MAG_OUT_X_L { get; }


        public abstract Task Initialise(string discoveredI2cDevice);

        public Gyroscope GetGyroscopeReadings()
        {
            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value)
            //Start at OUT_X_L_G and read 6 bytes.
            byte[] data = ReadBytesFromGyroscope(GYR_OUT_X_L_G, (byte)6);

            int gyroscopeRawX = (int)(data[0] | (data[1] << 8));
            int gyroscopeRawY = (int)(data[2] | (data[3] << 8));
            int gyroscopeRawZ = (int)(data[4] | (data[5] << 8));
            if (gyroscopeRawX >= 32768) gyroscopeRawX = gyroscopeRawX - 65536;
            if (gyroscopeRawY >= 32768) gyroscopeRawY = gyroscopeRawY - 65536;
            if (gyroscopeRawZ >= 32768) gyroscopeRawZ = gyroscopeRawZ - 65536;

            Gyroscope gyroscopeReadings;
            gyroscopeReadings.rawX = gyroscopeRawX;
            gyroscopeReadings.rawY = gyroscopeRawY;
            gyroscopeReadings.rawZ = gyroscopeRawZ;

            return gyroscopeReadings;
        }

        public Accelerometer GetAccelerometerReadings()
        {
            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value)
            //Start at OUT_X_L_A and read 6 bytes.

            byte[] data = ReadBytesFromAccelerometer(ACC_OUT_X_L, (byte)6);

            int accelerometerRawX = (int)(data[0] | (data[1] << 8));
            int accelerometerRawY = (int)(data[2] | (data[3] << 8));
            int accelerometerRawZ = (int)(data[4] | (data[5] << 8));
            if (accelerometerRawX >= 32768) accelerometerRawX = accelerometerRawX - 65536;
            if (accelerometerRawY >= 32768) accelerometerRawY = accelerometerRawY - 65536;
            if (accelerometerRawZ >= 32768) accelerometerRawZ = accelerometerRawZ - 65536;

            Accelerometer accelerometerReadings;
            accelerometerReadings.rawX = accelerometerRawX;
            accelerometerReadings.rawY = accelerometerRawY;
            accelerometerReadings.rawZ = accelerometerRawZ;

            return accelerometerReadings;
        }

        public Magnetometer GetMagnetometerReadings()
        {
            //Read the measurements from the sensors, combine and convert to correct values
            //The values are expressed in 2’s complement (MSB for the sign and then 15 bits for the value)
            //Start at OUT_X_L_A and read 6 bytes.

            byte[] data = ReadBytesFromMagnetometer(MAG_OUT_X_L, (byte)6);

            int magnetometerRawX = (int)(data[0] | (data[1] << 8));
            int magnetometerRawY = (int)(data[2] | (data[3] << 8));
            int magnetometerRawZ = (int)(data[4] | (data[5] << 8));
            if (magnetometerRawX >= 32768) magnetometerRawX = magnetometerRawX - 65536;
            if (magnetometerRawY >= 32768) magnetometerRawY = magnetometerRawY - 65536;
            if (magnetometerRawZ >= 32768) magnetometerRawZ = magnetometerRawZ - 65536;

            Magnetometer magnetometerReadings;
            magnetometerReadings.rawX = magnetometerRawX;
            magnetometerReadings.rawY = magnetometerRawY;
            magnetometerReadings.rawZ = magnetometerRawZ;
            magnetometerReadings.headingInDegrees = 0.0;
            magnetometerReadings.headingInDegreesTiltCompensated = 0.0;

            return magnetometerReadings;
        }

        // Read a series of bytes from the gyroscope
        protected byte[] ReadBytesFromGyroscope(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);  // The MSB is set as this is required by the LSM9DSO to auto increment when reading a series of bytes
            i2cDeviceGyroscope.WriteRead(buffer, values);
            return values;
        }

        // Read a series of bytes from the accelerometer
        protected byte[] ReadBytesFromAccelerometer(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);
            i2cDeviceAccelerometer.WriteRead(buffer, values);
            return values;
        }

        // Read a series of bytes from the magnetometer
        protected byte[] ReadBytesFromMagnetometer(byte regAddr, int length)
        {
            byte[] values = new byte[length];
            byte[] buffer = new byte[1];
            buffer[0] = (byte)(0x80 | regAddr);
            i2cDeviceMagnetometer.WriteRead(buffer, values);    // The magnetometer uses the same I2C slave address as the accelerometer
            return values;
        }

        // Write a byte to the gyroscope
        protected void WriteByteToGyroscope(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] writeBuf = new byte[] { regAddr, value };
            i2cDeviceGyroscope.Write(writeBuf);
        }

        // Write a byte to the accelerometer
        protected void WriteByteToAccelerometer(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] writeBuf = new byte[] { regAddr, value };
            i2cDeviceAccelerometer.Write(writeBuf);
        }

        // Write a byte to the magnetometer
        protected void WriteByteToMagnetometer(byte regAddr, byte value)
        {
            byte[] values = new byte[value];
            byte[] writeBuf = new byte[] { regAddr, value };
            i2cDeviceMagnetometer.Write(writeBuf);      // The magnetometer uses the same I2C slave address as the accelerometer
        }

        public void Dispose()
        {
            // Cleanup
            i2cDeviceGyroscope.Dispose();
            i2cDeviceAccelerometer.Dispose();
            i2cDeviceMagnetometer.Dispose();
        }


    }
}
