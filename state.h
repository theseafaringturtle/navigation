#include <signal.h>

#define RECALIBRATION_THRESHOLD 0.6

// This class stores the current state of the sensors and performs some checks/conversions
class State
{
public:
    double last_linear_vel_imu = 0;
    double last_angular_vel_imu = 0;
    double last_linear_vel_encoders = 0;
    double last_angular_vel_encoders = 0;

    unsigned long last_timestamp_imu = 0;
    unsigned long last_timestamp_encoders = 0;
    bool imu_calibrating = false;

    pid_t* imu_pid;
    pid_t* encoders_pid = 0;

    void read_LSM9DS1_data(void *message_buf);
    void read_encoder_data(void *message_buf);

    bool check_need_imu_calibration(LSM9DS1_Message* message);

    ~State();
};
