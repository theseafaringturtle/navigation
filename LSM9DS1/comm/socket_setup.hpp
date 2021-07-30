
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int setup_unix_socket(const char* path)
{
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

int wait_accept_socket(int sfd)
{
    fd_set input;
    // NOTE: blocks until a connection request arrives.
    FD_ZERO(&input);
    FD_SET(sfd, &input);

    int cfd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);
    if(cfd == -1) {
        if (errno == EWOULDBLOCK) {
        // printf("No pending connections; sleeping for one second.\n");
        return 0;
      } else {
        perror("error when accepting connection");
        return 0;
      }
    } else {
        return cfd;
    }
}