#include <atomic>
#include <functional>

#include "../comm/encoders_comm_data.h"
#include "sensor.hpp"

class Encoder_Sensors : Sensor {
   public:
    std::atomic<int> left_encoder_delta;
    std::atomic<int> right_encoder_delta;

    std::queue<Encoders_Message> m_queue;
    std::mutex m_mutex;

    Encoder_Sensors();
    virtual void producer_socket_loop(int sfd) override;
    virtual void run() override;
    void start_producer_socket(const char* path);

    void reader_thread(bool left);
    void thread_timer();
};