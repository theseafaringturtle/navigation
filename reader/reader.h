
#include "LSM9DS1_Types.h"
#include <wiringPi.h>
#include "LSM9DS1.h"

#define INT_PIN 4 // BCM is default now apparently
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

    inline static bool setupReader(LSM9DS1* imu) {
        IMUReader::imu = imu;
        IMUReader::current_reading = 0;
        if((wiringPiISR(INT_PIN, INT_EDGE_RISING, &gyro_isr) < 0)) {
            printf("Failed to set up interrupt\n");
            return 0;
        } else {
            printf("ISR set up\n");
            return 1;
        }
    }

    static void gyro_isr();
    
};