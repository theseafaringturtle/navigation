#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <functional>

#include "Encoder/dual_encoder.h"
#include "comm/imu_comm_data.h"
#include "comm/socket_setup.hpp"
#include "consumer.h"
#include "sensor_message_reader.h"
#include "state.h"

// create a new file for i2c script to check if it needs to restart
#define ENCODERS_RESTART_PATH "/home/pi/restart_encoders"
#define IMU_RESTART_PATH "/home/pi/restart_imu"


int main(int argc, char* argv[]) {

    int imu_sfd = setup_unix_socket(IMU_SOCK_PATH);
    if (!imu_sfd) {
        exit(EXIT_FAILURE);
    }
    int encoders_sfd = setup_unix_socket(E_SOCK_PATH);
    if (!encoders_sfd) {
        exit(EXIT_FAILURE);
    }
    State state;
    std::function<void(void*)> imu_callback = std::bind(&State::read_LSM9DS1_data, state, std::placeholders::_1);
    SensorMessageReader imu_reader(imu_sfd, IMU_RESTART_PATH, LSM9DS1_MESSAGE_SIZE, imu_callback);
    state.imu_pid = &imu_reader.producer_pid;
    imu_reader.start();

    std::function<void(void*)> encoder_callback = std::bind(&State::read_encoder_data, state, std::placeholders::_1);
    SensorMessageReader encoders_reader(encoders_sfd, ENCODERS_RESTART_PATH, ENCODERS_MESSAGE_SIZE, encoder_callback);
    state.encoders_pid = &encoders_reader.producer_pid;
    encoders_reader.start();

    getchar();
    imu_reader.stop();
    encoders_reader.stop();

    return 0;
}
