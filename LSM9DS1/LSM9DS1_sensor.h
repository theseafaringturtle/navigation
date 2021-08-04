#include <functional>

class LSM9DS1_Sensor {
   public:
    void consumer_signal(int sigcode);
    void has_sample(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);

    void producer_socket_loop(int sfd);
    void run();
    void start_producer_socket(char* path);
};
