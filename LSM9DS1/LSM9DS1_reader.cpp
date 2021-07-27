#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "library/LSM9DS1.h"
#include "library/LSM9DS1_Types.h"
#include "comm/comm.h"

using namespace LSM9DS1_SharedState;

class LSM9DS1SendCallback : public LSM9DS1callback {
    virtual void hasSample(float gx, float gy, float gz,
                           float ax, float ay, float az,
                           float mx, float my, float mz) {
        std::lock_guard<std::mutex> lock(m_mutex);
        LSM9DS1_Message gyro_message = {'G', gx, gy, gz};
        m_queue.push(gyro_message);
        LSM9DS1_Message accel_message = {'G', gx, gy, gz};
        m_queue.push(accel_message);
    }
};


int main(int argc, char *argv[]) {
    // Register signal handler so any process can ask for recalibration or shutdown
    struct sigaction consumer_action;
    consumer_action.sa_flags = 0;
    consumer_action.sa_handler = consumer_signal;
    sigemptyset(&consumer_action.sa_mask);
    sigaction(SIGUSR1, &consumer_action, NULL);
    sigaction(SIGUSR2, &consumer_action, NULL);
    sigaction(SIGINT, &consumer_action, NULL);
    // Start socket thread
    std::thread producer_UDS_thread(producer_loop);

    // Start IMU callback
    LSM9DS1 imu(IMU_MODE_I2C, 0x6b, 0x1e);
    // imu.configBDU(true, false);
    LSM9DS1SendCallback imu_callback;
    imu.setCallback(&imu_callback);
    imu.begin();

    while (imu_running) {
        if (recalibrating) {
            if (times_recalibrated == 5) {
                printf("Tried recalibrating, the i2c lines are too noisy\n");
                exit(EXIT_FAILURE);
            }
            printf("Recalibrating\n");
            usleep(20e3);
            imu.calibrate();
            times_recalibrated++;
            recalibrating = false;
        }
    }
    usleep(500);

    imu.end();
    producer_UDS_thread.join();

    exit(EXIT_SUCCESS);
}

