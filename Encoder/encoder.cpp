
#include <atomic>
#include <fcntl.h>
#include <linux/input.h>
#include <mutex>
#include <stdio.h>
#include <unistd.h>

#include "encoder.h"

Encoder_Sensors::Encoder_Sensors() {
    running = true;
    left_encoder_delta = 0;
    right_encoder_delta = 0;
}

void Encoder_Sensors::run()
{
    std::thread left_reader_thread(Encoder_Sensors::reader_thread, this, true);
    std::thread right_reader_thread(Encoder_Sensors::reader_thread, this, false);

    left_reader_thread.join();
    right_reader_thread.join();
}

void Encoder_Sensors::reader_thread(bool left) {
    struct input_event ievt;
    int ievt_size = sizeof(struct input_event);
    char* path;
    if(left)
        path = "/dev/input/by-path/platform-rotary@11-event";
    else
        path = "/dev/input/by-path/platform-rotary@17-event";
    int ifd = open(path, O_RDONLY);
    if (ifd == -1) {
        printf("cannot open input!\n");
        return;
    }
    int bytes_read;
    fd_set input;
    struct timeval timeout;

    while (running) {
        FD_ZERO(&input);
        FD_SET(ifd, &input);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        int ret = select(ifd + 1, &input, NULL, NULL, &timeout);
        if (!ret) {
            printf("Timed out\n");
            continue;
        } else if (ret == -1) {
            perror("Select");
            return;
        }
        bytes_read = read(ifd, &ievt, ievt_size);
        if (ievt.type == EV_REL && bytes_read == ievt_size) {
            if(left)
                left_encoder_delta += ievt.value;
            else
                right_encoder_delta += ievt.value;
        }
    }
}

//todo move queue into base class?
void Encoder_Sensors::producer_socket_loop(int sfd)
{
    for (size_t i = 0; i < m_queue.size(); i++)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        Encoders_Message message = m_queue.front();
        if (write(sfd, &message, ENCODERS_MESSAGE_SIZE) != ENCODERS_MESSAGE_SIZE)
        {
            printf("Failed to write to socket\n");
            close(sfd);
            exit(EXIT_FAILURE);
        }
        m_queue.pop();
    }
    usleep(1e3); // 1ms
}

void Encoder_Sensors::thread_timer() {
    // Make sure the compiler won't reuse the same variable
    volatile int l_delta = left_encoder_delta;
    volatile int r_delta = right_encoder_delta;
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    unsigned long timestamp_us = ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
    Encoders_Message message = {'E', timestamp_us, l_delta, r_delta};
    // Don't zero them out in case they've been updated in the meantime
    left_encoder_delta -= l_delta;
    right_encoder_delta -= r_delta;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(message);
}