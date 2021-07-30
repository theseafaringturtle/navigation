#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "comm.h"

void consumer_signal(int sigcode)
{
    if (sigcode == SIGUSR1) {
        LSM9DS1_SharedState::recalibrating = true;
    } else if (sigcode == SIGUSR2 || sigcode == SIGINT) { // todo reuse SIGUSR2?
        LSM9DS1_SharedState::imu_running = false;
    }
}

void producer_loop()
{
    struct sockaddr_un addr;

    int sfd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    printf("Client socket fd = %d\n", sfd);

    // Make sure socket's file descriptor is legit.
    if (sfd == -1) {
        perror("socket");
        LSM9DS1_SharedState::imu_running = false;
        return;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        LSM9DS1_SharedState::imu_running = false;
        return;
    }
    // write PID so we can receive signals
    pid_t pid = getpid();
    if (write(sfd, &pid, sizeof(pid_t)) != sizeof(pid_t)) {
        printf("Failed to write PID to socket\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    while (LSM9DS1_SharedState::imu_running) {
        for (size_t i = 0; i < LSM9DS1_SharedState::m_queue.size(); i++) {
            std::lock_guard<std::mutex> lock(LSM9DS1_SharedState::m_mutex);
            LSM9DS1_Message message = LSM9DS1_SharedState::m_queue.front();
            if (write(sfd, &message, MESSAGE_SIZE) != MESSAGE_SIZE) {
                printf("Failed to write to socket\n");
                close(sfd);
                exit(EXIT_FAILURE);
            }
            LSM9DS1_SharedState::m_queue.pop();
        }

        usleep(10e3);
    }
}