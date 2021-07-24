#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "LSM9DS1_Types.h"
#include "LSM9DS1.h"
#include <signal.h>

#include <pigpio.h>
#include "reader.h"
#include "delta_time.h"

struct timespec start, end;
struct timespec delta;

bool running = false;
bool recalibrating = false;
int times_recalibrated = 0;
bool interrupting = false;

void sigint_handler(int sigcode)
{
  if (sigcode == SIGINT)
  {
    running = false;
  }
}

void IMUReader::gyro_isr(int gpio, int level, uint32_t tick)
{
  if (level != RISING_EDGE || !running || recalibrating)
  {
    return;
  }
  while (interrupting || !imu->gyroAvailable())
  {
    usleep(100);
  }
  interrupting = true;

  if(IMUReader::current_reading == BUFFER_SIZE) {
    IMUReader::current_reading = 0;
  }

  uint32_t delta = tick - last_time;
  last_time = tick;
  if (delta < 0)
  { // tick integer wrap-around check
    return;
  }
  // uint8_t n_samples = imu->getFIFOSamples(); // not required since triggered on every sample
  // for (int i = 0; i < n_samples; i++)
  // {
  // printf("%i\n", n_samples);
  IMUReader::imu->readGyro();
  double roll = IMUReader::imu->calcGyro(IMUReader::imu->gx);
  double pitch = IMUReader::imu->calcGyro(IMUReader::imu->gy);
  double yaw = IMUReader::imu->calcGyro(IMUReader::imu->gz);
  if ((current_reading <= 2) && (roll > RECALIBRATION_THRESHOLD || pitch > RECALIBRATION_THRESHOLD || yaw > RECALIBRATION_THRESHOLD ||
                                 -roll < -RECALIBRATION_THRESHOLD || -pitch < -RECALIBRATION_THRESHOLD || -yaw < -RECALIBRATION_THRESHOLD))
  {
    recalibrating = true;
    interrupting = false;
    return;
  }
  IMUReader::delta_times[IMUReader::current_reading] = delta;
  IMUReader::gyro_readings[IMUReader::current_reading][0] = roll;
  IMUReader::gyro_readings[IMUReader::current_reading][1] = pitch;
  IMUReader::gyro_readings[IMUReader::current_reading][2] = yaw;
  IMUReader::current_reading++;
  // }

  interrupting = false;
}

int main(int argc, char *argv[])
{
  running = true;

  int status = gpioInitialise(); // note: this clears all signal handlers, keep it at the top
  if (status <= 0)
  {
    printf("Pigpio init failed\n");
    exit(EXIT_FAILURE);
  }
  // register CTRL+C handler
  struct sigaction ctrl_c_action;
  ctrl_c_action.sa_flags = 0;
  ctrl_c_action.sa_handler = sigint_handler;
  sigemptyset(&ctrl_c_action.sa_mask);
  sigaction(SIGINT, &ctrl_c_action, NULL);

  LSM9DS1 imu(IMU_MODE_I2C, 0x6b, 0x1e);

  imu.begin();
  if (!imu.begin())
  {
    fprintf(stderr, "Failed to communicate with LSM9DS1.\n");
    exit(EXIT_FAILURE);
  }

  printf("Calibrating\n");
  imu.calibrate();

  IMUReader::setupReader(&imu);

  imu.configBDU(true, false);

  imu.configInt(XG_INT1, INT_FTH, INT_ACTIVE_HIGH, INT_PUSH_PULL); // INT1_IG_G | INT_DRDY_G

  imu.enableFIFO(true);
  usleep(50e3);

  imu.setFIFO(FIFO_OFF, 0x15);
  imu.setFIFO(FIFO_CONT, 0x15);

  while (running)
  {
    if (recalibrating)
    {
      if (times_recalibrated == 5)
      {
        printf("Tried recalibrating, the i2c lines are too noisy\n");
        exit(EXIT_FAILURE);
      }
      printf("Recalibrating\n");
      usleep(20e3);
      imu.calibrate();
      times_recalibrated++;
      recalibrating = false;
    }
    usleep(100);
  }
  printf("Readings: %i\n", IMUReader::current_reading);

  IMUReader::store_readings(0, IMUReader::current_reading);

  exit(EXIT_SUCCESS);
}

void IMUReader::store_readings(int start, int end) {
  FILE *p = fopen("imu.txt", "w");
  char buffer[100];
  for (int i = start; i < end; i++)
  {
    snprintf(buffer, 100, "%f %f %f %ld\n", IMUReader::gyro_readings[i][0],
                       IMUReader::gyro_readings[i][1], IMUReader::gyro_readings[i][2], IMUReader::delta_times[i]);
    fputs(buffer, p);
  }
  fclose(p);
}