#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "comm.h"

using namespace LSM9DS1_SharedState;

void consumer_signal(int sigcode) {
    if (sigcode == SIGUSR1 || sigcode == SIGINT) {
        recalibrating = true;
    } else if (sigcode == SIGUSR2) {
        imu_running = false;
    }
}

void producer_loop() {
    struct sockaddr_un addr;

    int sfd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    printf("Client socket fd = %d\n", sfd);

    // Make sure socket's file descriptor is legit.
    if (sfd == -1) {
        perror("socket");
        imu_running = false;
        return;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        imu_running = false;
        return;
    }

    while (imu_running) {
        for (size_t i = 0; i < m_queue.size(); i++) {
            std::lock_guard<std::mutex> lock(m_mutex);
            LSM9DS1_Message message = m_queue.front();
            if (write(sfd, &message, MESSAGE_SIZE) != MESSAGE_SIZE) {
                printf("Failed to write to socket\n");
                exit(EXIT_FAILURE);
            }
            m_queue.pop();
        }
        
        usleep(10e3);
    }
}