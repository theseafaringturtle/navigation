
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <functional>

#define MESSAGE_SIZE 64 //sizeof(LSM9DS1_Message) 

// Set up listening socket
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

// Keep waiting for pending connections, return a descriptor when one is established 
int wait_accept_socket(int sfd) {
    fd_set input;
    // NOTE: blocks until a connection request arrives.
    FD_ZERO(&input);
    FD_SET(sfd, &input);

    int cfd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);
    if (cfd == -1) {
        if (errno == EWOULDBLOCK) { // no pending connections
            return 0;
        } else {
            perror("error when accepting connection");
            return 0;
        }
    } else {
        return cfd;
    }
}

pid_t read_producer_PID(int cfd) {
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

// Keep waiting for connections then reading messages in a loop using wait_accept_socket 
int read_unix_socket(int sfd, std::function<void (char*)> read_func, std::function<int (void)> read_failed_func) {

    ssize_t numRead;
    fd_set input;
    struct timeval timeout;

    char message_buf[MESSAGE_SIZE];
    int producer_pid;

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
            numRead = read(cfd, &message_buf, MESSAGE_SIZE);
            if (numRead != MESSAGE_SIZE) {
                perror("read failed");
                printf("numRead: %ld \n", numRead);
                if (!read_failed_func()) {
                    exit(EXIT_FAILURE);
                }
                break;
            } else {
                read_func(message_buf);
            }
        }

        if (close(cfd) == -1) {
            perror("close");
        }
    }
    return 0;
}