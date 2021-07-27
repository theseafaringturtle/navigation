#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "comm.h"

#define IMU_RESTART_PATH "/home/pi/restart_imu"  // create a new file for i2c script to check if it needs to restart

int main(int argc, char* argv[]) {
    int sfd = setup_unix_socket(SV_SOCK_PATH);
    if (!sfd) {
        exit(EXIT_FAILURE);
    }

    ssize_t numRead;
    bool producer_running = true;

    fd_set input;
    struct timeval timeout;

    LSM9DS1_Message message;

    while (producer_running) { /* Handle client connections iteratively */

        // sfd remains open and can be used to accept further connections. */
        printf("Waiting to accept a connection...\n");
        // NOTE: blocks until a connection request arrives.
        FD_ZERO(&input);
        FD_SET(sfd, &input);
        timeout.tv_sec = 10;
        int ret = select(sfd + 1, &input, NULL, NULL, &timeout);
        if (!ret) {
            printf("Timed out\n");
            continue;
        } else if (ret == -1) {
            perror("select");
        }
        int cfd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);
        if (cfd == -1) {
            perror("accept");
        }
        printf("Accepted socket fd = %d\n", cfd);

        while (true) {
            FD_ZERO(&input);
            FD_SET(cfd, &input);
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
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
                if(!write_restart_message()) {
                  exit(EXIT_FAILURE);
                }
            }

            printf("Message from %c\n", message.sensor);
        }

        if (close(cfd) == -1) {
            perror("close");
        }
    }
    return 0;
}

int write_restart_message() {
    int fd = open(IMU_RESTART_PATH, O_WRONLY | O_CREAT, 0600);
    if (!fd) {
        perror("restart");
        return 0;
    }
    close(fd);
    return 1;
}