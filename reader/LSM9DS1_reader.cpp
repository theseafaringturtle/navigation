#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "LSM9DS1_Types.h"
#include "LSM9DS1.h"

#include <wiringPi.h>
#include "reader.h"
#include "delta_time.h"

struct timespec start, end;
struct timespec delta;

bool running = false;
bool interrupting = false;

void IMUReader::gyro_isr()
{
  // printf("ISR called\n");
  if (!running)
  {
    return;
  }
  while(interrupting || !imu->gyroAvailable()) {
    usleep(100);
  }
  interrupting = true;
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  if (!timeval_subtract(&delta, &end, &start) && IMUReader::current_reading < BUFFER_SIZE) {
    // printf("Herp\n");
    // uint8_t n_samples = imu->getFIFOSamples();
    // for(int i=0; i<n_samples; i++) {
      // printf("%i\n", n_samples);
      IMUReader::imu->readGyro();
      IMUReader::delta_times[IMUReader::current_reading] = delta.tv_nsec / 1000;
      IMUReader::gyro_readings[IMUReader::current_reading][0] = IMUReader::imu->calcGyro(IMUReader::imu->gx);
      IMUReader::gyro_readings[IMUReader::current_reading][1] = IMUReader::imu->calcGyro(IMUReader::imu->gy);
      IMUReader::gyro_readings[IMUReader::current_reading][2] = IMUReader::imu->calcGyro(IMUReader::imu->gz);
      IMUReader::current_reading++;
    // }
  } else {
    printf("Derp\n");
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  interrupting = false;
}

int main(int argc, char *argv[])
{

  wiringPiSetup();

  LSM9DS1 imu(IMU_MODE_I2C, 0x6b, 0x1e);

  imu.begin();
  if (!imu.begin())
  {
    fprintf(stderr, "Failed to communicate with LSM9DS1.\n");
    exit(EXIT_FAILURE);
  }

  printf("Calibrating\n");
  imu.calibrate();

  if(!IMUReader::setupReader(&imu)){
    exit(EXIT_FAILURE);
  }

  // imu.configGyroInt(ZHIE_G, false, false);
  // imu.configGyroThs(5, Z_AXIS, 10, true);
  // usleep(3e6);


  imu.configBDU(false, false);

  imu.configInt(XG_INT1, INT_FTH, INT_ACTIVE_HIGH, INT_PUSH_PULL); // INT1_IG_G | INT_DRDY_G

  imu.enableFIFO(true);
  usleep(50e3);

  imu.setFIFO(FIFO_OFF, 0x10); 
  imu.setFIFO(FIFO_CONT, 0x10);



  // printf("Resetting\n");
  // imu.softReset();
  // usleep(1e6);
  // printf("Reset bit written\n");

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  printf("Waiting\n");
  running = true;
  getchar();
  running = false;
  printf("Readings: %i\n", IMUReader::current_reading);

  FILE *p = fopen("imu.txt", "w");
  char buffer[100];
  for (int i = 0; i < IMUReader::current_reading; i++)
  {
    int res = snprintf(buffer, 100, "%f %f %f %ld\n", IMUReader::gyro_readings[i][0], 
      IMUReader::gyro_readings[i][1], IMUReader::gyro_readings[i][2], IMUReader::delta_times[i]);
    if (res < 0 && res >= 100)
    {
      printf("Failed to snprintf to buffer\n");
    }
    fputs(buffer, p);
  }
  fclose(p);

  exit(EXIT_SUCCESS);
}
