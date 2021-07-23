#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "LSM9DS1_Types.h"
#include "LSM9DS1.h"


int timeval_subtract (struct timespec *result, struct timespec *x, struct timespec *y) {
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int nsec = (y->tv_nsec - x->tv_nsec) / 1e9 + 1;
    y->tv_nsec -= 1e9 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_nsec - y->tv_nsec > 1e9) {
    int nsec = (x->tv_nsec - y->tv_nsec) / 1e9;
    y->tv_nsec += 1e9 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_nsec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}


int main(int argc, char *argv[]) {

    // sleep time
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 800;

    // time delta vars
    struct timespec start,end;
    struct timespec result;

    // self-explanatory
    LSM9DS1 imu(IMU_MODE_I2C, 0x6b, 0x1e);
    // printf("Beginning\n");
    imu.begin();
    if (!imu.begin()) {
        fprintf(stderr, "Failed to communicate with LSM9DS1.\n");
        exit(EXIT_FAILURE);
    }
    // printf("Calibrating\n");
    imu.calibrate();

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    // printf("Starting loop\n");
    for (;;) {
        while (!imu.gyroAvailable());
        imu.readGyro();
        // while(!imu.accelAvailable()) ;
        // imu.readAccel();
        // while(!imu.magAvailable()) ;
        // imu.readMag();
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        if(timeval_subtract(&result, &end, &start)){
            printf("Negative delta\n");
        }
        else if(result.tv_sec > 1){
            printf("Excessive delay\n");
        }
        else {
            printf("%f, %f, %f, %ld\n", imu.calcGyro(imu.gx), imu.calcGyro(imu.gy), imu.calcGyro(imu.gz), result.tv_nsec / 1000);
        }
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        // printf("Accel: %f, %f, %f [Gs]\n", imu.calcAccel(imu.ax), imu.calcAccel(imu.ay), imu.calcAccel(imu.az));
        // printf("Mag: %f, %f, %f [gauss]\n", imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
        // usleep(1000); // 1ms
        select(0, NULL, NULL, NULL, &timeout);

    }

    exit(EXIT_SUCCESS);
}
