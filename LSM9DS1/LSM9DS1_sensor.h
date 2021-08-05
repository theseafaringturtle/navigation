#include <functional>
#include <atomic>
#include "../comm/imu_comm_data.h"
#include "sensor.hpp"

class LSM9DS1_Sensor : Sensor {
   public:
   LSM9DS1_Sensor();
   // LSM9DS1-specific state variables, other sensor may not need calibration
    std::atomic<bool> recalibrating;
    std::queue<LSM9DS1_Message> m_queue;
    std::mutex m_mutex;
    std::atomic<int> times_recalibrated;

    void consumer_signal(int sigcode);
    void has_sample(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

    virtual void producer_socket_loop(int sfd) override;
    virtual void run() override;
    void start_producer_socket(const char* path);
};
