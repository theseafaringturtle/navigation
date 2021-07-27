#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

#include "comm_data.h"

// Set up Unix datagram socket
int setup_unix_socket(const char *path);

// Got to keep them global since they're used in a signal handler
class LSM9DS1_SharedState
{
public:
    static std::atomic<bool> imu_running;
    static std::atomic<bool> recalibrating;

    static std::queue<LSM9DS1_Message> m_queue;
    static std::mutex m_mutex;
    static int times_recalibrated;
}; // namespace LSM9DS1_SharedState

void consumer_signal(int sigcode);

void producer_loop();

#define RECALIBRATION_THRESHOLD 0.6
