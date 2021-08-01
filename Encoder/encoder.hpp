
#include <atomic>
#include <fcntl.h>
#include <linux/input.h>
#include <mutex>
#include <stdio.h>
#include <unistd.h>

std::atomic<bool> reading_encoder(true);
std::atomic<int> encoder_position(0);

void encoder_loop(void)
{
    struct input_event ievt;
    int ievt_size = sizeof(struct input_event);
    int ifd = open("/dev/input/by-path/platform-rotary@11-event", O_RDONLY); // a = 17, b = 18
    if (ifd == -1) {
        printf("cannot open input!\n");
        return;
    }
    int ird;

    fd_set input;
    struct timeval timeout;

    while (reading_encoder) {
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
        ird = read(ifd, &ievt, ievt_size);
        if (ievt.type == EV_REL && ird == ievt_size) {
            // printf("%i\n", ievt.value);
            encoder_position = (encoder_position + ievt.value) % 600;
        }
    }
}