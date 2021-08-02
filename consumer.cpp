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

#include "Encoder/encoder.hpp"
#include "LSM9DS1/comm/comm_data.h"
#include "LSM9DS1/comm/socket_setup.hpp"

#define IMU_RESTART_PATH "/home/pi/restart_imu"  // create a new file for i2c script to check if it needs to restart
#define RECALIBRATION_THRESHOLD 0.6

int current_reading = 0;
bool calibrating = false;

int write_restart_message(void) {
    int fd = open(IMU_RESTART_PATH, O_WRONLY | O_CREAT, 0600);
    if (!fd) {
        perror("restart");
        return 0;
    }
    close(fd);
    return 1;
}

void read_LSM9DS1_data(void* message_buf) {
    LSM9DS1_Message* message = static_cast<LSM9DS1_Message*>(message_buf);
    if (message->sensor == 'G') {
        if (current_reading < 5 && (message->reading_z > RECALIBRATION_THRESHOLD || -message->reading_z < RECALIBRATION_THRESHOLD)) {
            calibrating = true;
            // kill(producer_pid, SIGUSR1);
        }
        current_reading++;
        printf("Message from %c: %f\n", message->sensor, message->reading_z);
    } else if (message->sensor == 'A') {
        printf("Message from %c: %f\n", message->sensor, message->reading_x);
        current_reading++;
    }
    printf("Current encoder position: %i\n", encoder_position.load());
}

int main(int argc, char* argv[]) {
    std::thread encoder_thread(encoder_loop);

    int sfd = setup_unix_socket(SV_SOCK_PATH);
    if (!sfd) {
        exit(EXIT_FAILURE);
    }
    read_unix_socket(sfd, read_LSM9DS1_data, write_restart_message);

    reading_encoder = false;
    encoder_thread.join();
    return 0;
}
