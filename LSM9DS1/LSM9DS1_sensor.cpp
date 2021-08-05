#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "LSM9DS1_utils.h"
#include "LSM9DS1_sensor.h"

LSM9DS1_Sensor::LSM9DS1_Sensor()
{
    // Initialise atomic variables
    running = true;
    recalibrating = false;
    times_recalibrated = 0;
    // Register signal handler so any process can ask for recalibration or shutdown
    struct sigaction consumer_action;
    consumer_action.sa_flags = 0;
    LSM9DS1_SignalDIspatcher dispatcher;
    dispatcher.set_sensor(this);
    consumer_action.sa_handler = LSM9DS1_SignalDIspatcher::consumer_signal;
    sigemptyset(&consumer_action.sa_mask);
    sigaction(SIGUSR1, &consumer_action, NULL);
    sigaction(SIGUSR2, &consumer_action, NULL);
    sigaction(SIGINT, &consumer_action, NULL);
}

void LSM9DS1_Sensor::producer_socket_loop(int sfd)
{
    for (size_t i = 0; i < m_queue.size(); i++)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        LSM9DS1_Message message = m_queue.front();
        if (write(sfd, &message, LSM9DS1_MESSAGE_SIZE) != LSM9DS1_MESSAGE_SIZE)
        {
            printf("Failed to write to socket\n");
            close(sfd);
            exit(EXIT_FAILURE);
        }
        m_queue.pop();
    }

    usleep(1e3); // 1ms
}

void LSM9DS1_Sensor::consumer_signal(int sigcode)
{
    if (sigcode == SIGUSR1)
    {
        recalibrating = true;
    }
    else if (sigcode == SIGUSR2 || sigcode == SIGINT)
    { // todo reuse SIGUSR2?
        running = false;
    }
}

void LSM9DS1_Sensor::has_sample(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    unsigned long timestamp_us = (unsigned long) (ts.tv_sec * 1e6) + (unsigned long) (ts.tv_nsec / 1e3);
    std::lock_guard<std::mutex> lock(m_mutex);
    LSM9DS1_Message xg_message = {'L', timestamp_us, ax, gz};
    m_queue.push(xg_message);
}

void LSM9DS1_Sensor::run()
{
    // Start socket thread
    std::thread producer_UDS_thread(&Sensor::start_producer_socket, (Sensor*)this, IMU_SOCK_PATH);

    // Start IMU callback
    LSM9DS1 imu(IMU_MODE_I2C, 0x6b, 0x1e);
    imu.configBDU(true, false);
    LSM9DS1SendCallback imu_callback;
    imu_callback.set_sensor(this);
    imu.setCallback(&imu_callback);
    try
    {
        imu.begin();
    }
    catch (const char *errorMessage)
    {
        printf("%s\n", errorMessage);
        running = false;
    }

    while (running)
    {
        if (recalibrating)
        {
            if (times_recalibrated == 5)
            {
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

    imu.end();
    producer_UDS_thread.join();
}
