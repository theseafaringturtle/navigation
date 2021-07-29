#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "LSM9DS1/comm/comm_data.h"

#define IMU_RESTART_PATH "/home/pi/restart_imu"  // create a new file for i2c script to check if it needs to restart


int setup_unix_socket(const char* path) {
    struct sockaddr_un addr;
    // SEQPACKET preserves both the data frame and the ordering
    int sfd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    printf("Server socket fd = %d\n", sfd);

    // Make sure socket's file descriptor is legit.
    if (sfd == -1) {
        perror("Socket FD error\n");
        return 0;
    }

    // Delete any file that already exists at the address. Make sure the deletion
    // succeeds. If the error is just that the file/directory doesn't exist, it's
    // fine.
    if (remove(path) == -1 && errno != ENOENT) {
        perror("removing previous file");
        return 0;
    }

    // Zero out the address, and set family and path.
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        return 0;
    }

    if (listen(sfd, 1) == -1) {
        perror("listen");
        return 0;
    }
    return sfd;
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


int main(int argc, char* argv[]) {
    int sfd = setup_unix_socket(SV_SOCK_PATH);
    if (!sfd) {
        exit(EXIT_FAILURE);
    }

    ssize_t numRead;
    int current_reading = 0;
    bool producer_running = true;
    bool calibrating = false;

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
        
        int cfd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);
        if (cfd == -1 && errno != EAGAIN) {
            perror("accept");
            sleep(1);
            continue;
        }

        FD_ZERO(&input);
        FD_SET(cfd, &input);
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        int ret = select(cfd + 1, &input, NULL, NULL, &timeout);
        if (!ret) {
            printf("Timed out\n");
            continue;
        } else if (ret == -1) {
            perror("select");
            sleep(1);
            continue;
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
                break;
            } else {
                if(message.sensor == 'G') {
                    if(current_reading < 5 && (message.reading_z > RECALIBRATION_THRESHOLD || -message.reading_z < RECALIBRATION_THRESHOLD)) {
                        calibrating = true;
                        //use another socket for sending? maybe useful for changing params on the go?
                    }
                    printf("Message from %c: %f\n", message.sensor, message.reading_z);
                }
                else if(message.sensor == 'A') {
                    printf("Message from %c: %f\n", message.sensor, message.reading_x);
                }
            }

        }

        if (close(cfd) == -1) {
            perror("close");
        }
    }
    return 0;
}
