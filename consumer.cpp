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

#define ENCODERS_RESTART_PATH "/home/pi/restart_encoders"
#define IMU_RESTART_PATH "/home/pi/restart_imu"  // create a new file for i2c script to check if it needs to restart
#define RECALIBRATION_THRESHOLD 0.6

int current_reading = 0;
bool calibrating = false;

int write_restart_message_imu(void) {
    int fd = open(IMU_RESTART_PATH, O_WRONLY | O_CREAT, 0600);
    if (!fd) {
        perror("restart");
        return 0;
    }
    close(fd);
    return 1;
}

int write_restart_message_encoders(void) {
    int fd = open(ENCODERS_RESTART_PATH, O_WRONLY | O_CREAT, 0600);
    if (!fd) {
        perror("restart");
        return 0;
    }
    close(fd);
    return 1;
}

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

    // TODO clean exit
    std::thread imu_reader_thread (&read_unix_socket, imu_sfd, read_LSM9DS1_data, write_restart_message_imu, LSM9DS1_MESSAGE_SIZE);
    std::thread encoder_reader_thread(&read_unix_socket, encoders_sfd, read_encoder_data, write_restart_message_encoders, ENCODERS_MESSAGE_SIZE);

    imu_reader_thread.join();
    encoder_reader_thread.join();

    return 0;
}
