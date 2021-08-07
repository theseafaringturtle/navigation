#include <functional>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>

#include "sensor_message_reader.h"

SensorMessageReader::SensorMessageReader(int file_desc, const char *restart_path, size_t message_size, std::function<void(void *)> read_func)
{
    producer_pid = 0;
    this->restart_path = restart_path;
    this->read_func = read_func;
    this->sfd = file_desc;
    this->running = true;
    this->message_size = message_size;
}

void SensorMessageReader::start()
{
    _thread = std::thread(&SensorMessageReader::read_unix_socket, this);
}

void SensorMessageReader::stop()
{
    running = false;
    _thread.join();
}
pid_t SensorMessageReader::get_producer_pid()
{
    return producer_pid;
}

int SensorMessageReader::write_restart_message()
{
    int fd = open(restart_path, O_WRONLY | O_CREAT, 0600);
    if (!fd)
    {
        perror("restart");
        return 0;
    }
    close(fd);
    return 1;
}

// Keep waiting for pending connections, return a descriptor when one is established
int SensorMessageReader::wait_accept_socket()
{
    fd_set input;
    // NOTE: blocks until a connection request arrives.
    FD_ZERO(&input);
    FD_SET(sfd, &input);

    int cfd = accept4(sfd, NULL, NULL, SOCK_NONBLOCK);
    if (cfd == -1)
    {
        if (errno == EWOULDBLOCK)
        { // no pending connections
            return 0;
        }
        else
        {
            perror("error when accepting connection");
            return 0;
        }
    }
    else
    {
        return cfd;
    }
}

pid_t SensorMessageReader::read_producer_PID(int cfd)
{
    fd_set input;
    struct timeval timeout;
    FD_ZERO(&input);
    FD_SET(cfd, &input);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100e3;
    int ret = select(cfd + 1, &input, NULL, NULL, &timeout);
    if (!ret)
    {
        printf("PID read timed out\n");
        return 0;
    }
    else if (ret == -1)
    {
        perror("Select");
        return 0;
    }
    int PID;
    pid_t numRead = read(cfd, &PID, sizeof(pid_t));
    if (numRead != sizeof(pid_t))
    {
        perror("Read failed");
        return 0;
    }
    return PID;
}

// Keep waiting for connections then reading messages in a loop using wait_accept_socket
int SensorMessageReader::read_unix_socket()
{

    size_t numRead;
    fd_set input;
    struct timeval timeout;

    char message_buf[message_size];

    while (running)
    {
        // sfd remains open and can be used to accept further connections. */
        printf("Waiting to accept a connection...\n");
        int cfd = wait_accept_socket();
        if (!cfd)
        {
            sleep(1);
            continue;
        }
        printf("Accepted socket fd = %d\n", cfd);
        this->producer_pid = read_producer_PID(cfd);
        if (!producer_pid)
        {
            close(cfd);
            continue;
        }

        while (running)
        {
            FD_ZERO(&input);
            FD_SET(cfd, &input);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100e3;
            int ret = select(cfd + 1, &input, NULL, NULL, &timeout);
            if (!ret)
            {
                printf("socket read timed out\n");
                continue;
            }
            else if (ret == -1)
            {
                perror("Select");
                return 1;
            }
            numRead = read(cfd, &message_buf, message_size);
            if (numRead != message_size)
            {
                perror("read failed");
                printf("numRead: %u \n", numRead);
                if (!this->write_restart_message())
                {
                    exit(EXIT_FAILURE);
                }
                break;
            }
            else
            {
                read_func(message_buf);
            }
        }

        if (close(cfd) == -1)
        {
            perror("close");
        }
    }
    return 0;
}
