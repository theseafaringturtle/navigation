#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

// todo move to separate header
struct LSM9DS1_Message {
    char sensor;
    double reading_x;
    double reading_y;
    double reading_z;
};
#define MESSAGE_SIZE sizeof(LSM9DS1_Message)

#define SV_SOCK_PATH "/home/pi/uds"

// Set up Unix datagram socket
int setup_unix_socket(const char* path);
int write_restart_message();

// Got to keep them global since they're used in a signal handler
namespace LSM9DS1_SharedState {
extern std::atomic<bool> imu_running;
extern std::atomic<bool> recalibrating;

extern std::queue<LSM9DS1_Message> m_queue;
extern std::mutex m_mutex;

extern int times_recalibrated;
};  // namespace LSM9DS1_SharedState

void consumer_signal(int sigcode);

void producer_loop();

#define RECALIBRATION_THRESHOLD 0.6
