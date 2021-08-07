#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>

#include "Encoder/dual_encoder.h"
#include "comm/imu_comm_data.h"
#include "comm/socket_setup.hpp"
#include "consumer.h"
#include "sensor_message_reader.h"

#define ENCODERS_RESTART_PATH "/home/pi/restart_encoders"
#define IMU_RESTART_PATH "/home/pi/restart_imu"  // create a new file for i2c script to check if it needs to restart
#define RECALIBRATION_THRESHOLD 0.6

int current_reading = 0;
bool calibrating = false;


void read_LSM9DS1_data(void* message_buf) {
    LSM9DS1_Message* message = static_cast<LSM9DS1_Message*>(message_buf);
    if (current_reading < 5 && (message->gyro_z > RECALIBRATION_THRESHOLD || -message->gyro_z < RECALIBRATION_THRESHOLD)) {
        calibrating = true;
        // kill(producer_pid, SIGUSR1);
    }
    printf("Message from %c: %f at time %lu\n", message->sensor, message->acc_x, message->timestamp);
    current_reading++;
}

void read_encoder_data(void* message_buf) {
    Encoders_Message* message = static_cast<Encoders_Message*>(message_buf);
    printf("Message from %c: %i at time %lu\n", message->sensor, message->left_ticks, message->timestamp);
    current_reading++;
}

int main(int argc, char* argv[]) {

    int imu_sfd = setup_unix_socket(IMU_SOCK_PATH);
    if (!imu_sfd) {
        exit(EXIT_FAILURE);
    }
    int encoders_sfd = setup_unix_socket(E_SOCK_PATH);
    if (!encoders_sfd) {
        exit(EXIT_FAILURE);
    }

    SensorMessageReader imu_reader(imu_sfd, IMU_RESTART_PATH, LSM9DS1_MESSAGE_SIZE, read_LSM9DS1_data);
    imu_reader.start();
    SensorMessageReader encoders_reader(encoders_sfd, ENCODERS_RESTART_PATH, ENCODERS_MESSAGE_SIZE, read_encoder_data);
    encoders_reader.start();
    getchar();
    imu_reader.stop();
    encoders_reader.stop();

    return 0;
}
