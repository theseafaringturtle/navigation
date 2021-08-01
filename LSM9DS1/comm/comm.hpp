
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

#include "comm_data.h"

std::atomic<bool> imu_running(true);
std::atomic<bool> recalibrating(false);

std::queue<LSM9DS1_Message> m_queue;
std::mutex m_mutex;
std::atomic<int> times_recalibrated (0);

void consumer_signal(int sigcode)
{
    if (sigcode == SIGUSR1) {
        recalibrating = true;
    } else if (sigcode == SIGUSR2 || sigcode == SIGINT) { // todo reuse SIGUSR2?
        imu_running = false;
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
    // write PID so we can receive signals
    pid_t pid = getpid();
    if (write(sfd, &pid, sizeof(pid_t)) != sizeof(pid_t)) {
        printf("Failed to write PID to socket\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    while (imu_running) {
        for (size_t i = 0; i < m_queue.size(); i++) {
            std::lock_guard<std::mutex> lock(m_mutex);
            LSM9DS1_Message message = m_queue.front();
            if (write(sfd, &message, MESSAGE_SIZE) != MESSAGE_SIZE) {
                printf("Failed to write to socket\n");
                close(sfd);
                exit(EXIT_FAILURE);
            }
            m_queue.pop();
        }

        usleep(10e3);
    }
}
