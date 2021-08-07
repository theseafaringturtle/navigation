#include <functional>
#include <thread>

class SensorMessageReader
{
private:
    std::thread _thread;
    int producer_pid;
    const char *restart_path;
    int sfd;
    std::function<void(char *)> read_func;
    bool running;
    size_t message_size;

    int write_restart_message();
    int wait_accept_socket();
    pid_t read_producer_PID(int cfd);
    int read_unix_socket();
public:
    SensorMessageReader(int file_desc, const char *restart_path, size_t message_size, std::function<void(void *)> read_func);
    void start();
    void stop();
    pid_t get_producer_pid();
};
