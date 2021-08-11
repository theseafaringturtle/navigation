#include <stdio.h>

#include "comm/encoders_comm_data.h"
#include "comm/imu_comm_data.h"
#include "state.h"

void State::read_LSM9DS1_data(void *message_buf)
{
    LSM9DS1_Message *message = static_cast<LSM9DS1_Message *>(message_buf);
    if (this->check_need_imu_calibration(message))
    {
        if(*imu_pid) {
            imu_calibrating = true;
            kill(*imu_pid, SIGUSR1);
        } else {
            printf("Could not trigger recalibration: missing PID\n");
        }
    }
    printf("Message from %c: %f at time %lu\n", message->sensor, message->acc_x, message->timestamp);
}

void State::read_encoder_data(void *message_buf)
{
    Encoders_Message *message = static_cast<Encoders_Message *>(message_buf);
    printf("Message from %c: %i at time %lu\n", message->sensor, message->left_ticks, message->timestamp);
}

bool State::check_need_imu_calibration(LSM9DS1_Message *message)
{
    return message->timestamp - last_timestamp_imu > 5e6 && (message->acc_x > RECALIBRATION_THRESHOLD || -message->gyro_z < RECALIBRATION_THRESHOLD);
}

State::~State() {
    delete imu_pid;
    delete encoders_pid;
}