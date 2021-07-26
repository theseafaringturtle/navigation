#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "comm.h"

void consumer_signal(int sigcode) {
    if (sigcode == SIGUSR1) {
        // recalibrating = true;
    } else if (sigcode == SIGUSR2) {
        // running = false;
    }
}

int main(int argc, char* argv[]) {
    // register CTRL+C handler
    struct sigaction consumer_action;
    consumer_action.sa_flags = 0;
    consumer_action.sa_handler = consumer_signal;
    sigemptyset(&consumer_action.sa_mask);
    sigaction(SIGUSR1, &consumer_action, NULL);
    sigaction(SIGUSR2, &consumer_action, NULL);

    // move to thread and use 2
    struct sockaddr_un addr;

    int sfd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    printf("Client socket fd = %d\n", sfd);

    // Make sure socket's file descriptor is legit.
    if (sfd == -1) {
        perror("socket");
        return 0;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        return 0;
    }

    char buf[MESSAGE_SIZE];
    compose_message(buf);

    while (true) {
        if (write(sfd, buf, MESSAGE_SIZE) != MESSAGE_SIZE) {
            printf("partial/failed write\n");
            exit(EXIT_FAILURE);
        }
        usleep(10e3);
    }

    // Closes our socket; server sees EOF.
    exit(EXIT_SUCCESS);
}

void compose_message(char* buf) {
    char header = 'G';
    *buf = header;
    double roll = 0;
    double pitch = 0;
    double yaw = 0;
    int delta = 0;
    *(buf + sizeof(header)) = roll;
    *(buf + sizeof(header) + sizeof(double)) = pitch;
    *(buf + sizeof(header) + sizeof(double) * 2) = yaw;
    *(buf + sizeof(header) + sizeof(double) * 3) = delta;
}