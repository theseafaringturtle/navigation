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

#include "LSM9DS1/comm/comm_data.h"
#include "LSM9DS1/comm/socket_setup.hpp"

#include "Encoder/encoder.hpp"

#define IMU_RESTART_PATH "/home/pi/restart_imu" // create a new file for i2c script to check if it needs to restart
#define RECALIBRATION_THRESHOLD 0.6


int write_restart_message()
{
    int fd = open(IMU_RESTART_PATH, O_WRONLY | O_CREAT, 0600);
    if (!fd) {
        perror("restart");
        return 0;
    }
    close(fd);
    return 1;
}

pid_t read_producer_PID(int cfd)
{
    fd_set input;
    struct timeval timeout;
    FD_ZERO(&input);
    FD_SET(cfd, &input);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100e3;
    int ret = select(cfd + 1, &input, NULL, NULL, &timeout);
    if (!ret) {
        printf("Timed out\n");
        return 0;
    } else if (ret == -1) {
        perror("Select");
        return 0;
    }
    int PID;
    pid_t numRead = read(cfd, &PID, sizeof(pid_t));
    if (numRead != sizeof(pid_t)) {
        perror("Read failed");
        return 0;
    }
    return PID;
}

int main(int argc, char* argv[])
{
    int sfd = setup_unix_socket(SV_SOCK_PATH);
    if (!sfd) {
        exit(EXIT_FAILURE);
    }

    std::thread encoder_thread(encoder_loop);

    ssize_t numRead;
    int current_reading = 0;
    bool calibrating = false;

    fd_set input;
    struct timeval timeout;

    int producer_pid;
    LSM9DS1_Message message;

    while (true) {
        // sfd remains open and can be used to accept further connections. */
        printf("Waiting to accept a connection...\n");
        int cfd = wait_accept_socket(sfd);
        if (!cfd) {
            sleep(1);
            continue;
        }
        printf("Accepted socket fd = %d\n", cfd);
        producer_pid = read_producer_PID(cfd);
        if (!producer_pid) {
            close(cfd);
            continue;
        }

        while (true) {
            FD_ZERO(&input);
            FD_SET(cfd, &input);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100e3;
            int ret = select(cfd + 1, &input, NULL, NULL, &timeout);
            if (!ret) {
                printf("Timed out\n");
                continue;
            } else if (ret == -1) {
                perror("Select");
                return 1;
            }
            numRead = read(cfd, &message, MESSAGE_SIZE);
            if (numRead != MESSAGE_SIZE) {
                perror("read failed");
                printf("numRead: %i \n", numRead);
                if (!write_restart_message()) {
                    exit(EXIT_FAILURE);
                }
                break;
            } else {
                if (message.sensor == 'G') {
                    if (current_reading < 5 && (message.reading_z > RECALIBRATION_THRESHOLD || -message.reading_z < RECALIBRATION_THRESHOLD)) {
                        calibrating = true;
                        // kill(producer_pid, SIGUSR1);
                    }
                    current_reading++;
                    printf("Message from %c: %f\n", message.sensor, message.reading_z);
                } else if (message.sensor == 'A') {
                    printf("Message from %c: %f\n", message.sensor, message.reading_x);
                    current_reading++;
                }
                printf("Current encoder position: %i\n", encoder_position.load());
            }
        }

        if (close(cfd) == -1) {
            perror("close");
        }
    }
    reading_encoder = false;
    encoder_thread.join();
    return 0;
}
