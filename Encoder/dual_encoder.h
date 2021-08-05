#include <atomic>
#include <functional>

#include "../comm/encoders_comm_data.h"
#include "../comm/sensor.hpp"
#include "../CppTimer.h"

class DualEncoder_Sensor : Sensor
{
public:
    std::atomic<int> left_encoder_delta;
    std::atomic<int> right_encoder_delta;

    std::queue<Encoders_Message> m_queue;
    std::mutex m_mutex;

    DualEncoder_Sensor();
    virtual void producer_socket_loop(int sfd) override;
    virtual void run() override;
    void start_producer_socket(const char *path);

    void reader_thread(bool left);
    void enqueue_message();
};

class Encoder_Timer : public CppTimer
{
public:
    DualEncoder_Sensor* _sensor;
    Encoder_Timer(DualEncoder_Sensor* sensor);
    void timerEvent();
};