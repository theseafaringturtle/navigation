
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>

// All sensors should inherit from this
class Sensor {
   public:
    std::atomic<bool> running;
    virtual void producer_socket_loop(int sfd) = 0;
    virtual void run() = 0;

    void start_producer_socket(const char* path) {
        struct sockaddr_un addr;

        int sfd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
        printf("Client socket fd = %d\n", sfd);

        // Make sure socket's file descriptor is legit.
        if (sfd == -1) {
            perror("socket");
            running = false;
            return;
        }

        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

        if (connect(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
            perror("connect");
            running = false;
            return;
        }
        // write PID so we can receive signals
        pid_t pid = getpid();
        if (write(sfd, &pid, sizeof(pid_t)) != sizeof(pid_t)) {
            printf("Failed to write PID to socket\n");
            close(sfd);
            exit(EXIT_FAILURE);
        }

        while (running) {
            producer_socket_loop(sfd);
        }
    }
};
