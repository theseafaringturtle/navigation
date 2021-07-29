#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

#include "comm_data.h"

// Got to keep them global since they're used in a signal handler
class LSM9DS1_SharedState {
public:
    inline static std::atomic<bool> imu_running = true;
    inline static std::atomic<bool> recalibrating = false;

    inline static std::queue<LSM9DS1_Message> m_queue;
    inline static std::mutex m_mutex;
    inline static int times_recalibrated = 0;
}; // namespace LSM9DS1_SharedState

void consumer_signal(int sigcode);

void producer_loop();
