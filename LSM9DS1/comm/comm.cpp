
#include "comm.h"

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

std::atomic<bool> LSM9DS1_SharedState::imu_running (true);
std::atomic<bool> LSM9DS1_SharedState::recalibrating (false);
std::queue<LSM9DS1_Message> LSM9DS1_SharedState::m_queue;
std::mutex LSM9DS1_SharedState::m_mutex;
int LSM9DS1_SharedState::times_recalibrated = 0;