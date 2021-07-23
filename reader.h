
#include "LSM9DS1_Types.h"
#include "LSM9DS1.h"

#define INT_PIN 7
#define IGNORE_CHANGE_BELOW_USEC 500

// void gyro_isr(LSM9DS1* imu);

#define BUFFER_SIZE 4096 // write to file later, print at the end for now


class IMUReader
{
public:
    inline static LSM9DS1* imu;
    inline static double gyro_readings[BUFFER_SIZE][3]; // pitch, yaw, roll
    inline static unsigned long delta_times[BUFFER_SIZE];
    inline static int current_reading;

    inline IMUReader(LSM9DS1* imu) {
        IMUReader::imu = imu;
        IMUReader::current_reading = 0;
    }

    static void gyro_isr();
    
};