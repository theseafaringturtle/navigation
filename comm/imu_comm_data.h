struct LSM9DS1_Message {
    char sensor;
    unsigned long timestamp;
    double acc_x;
    double gyro_z;
};

#define LSM9DS1_MESSAGE_SIZE sizeof(LSM9DS1_Message) 

#define IMU_SOCK_PATH "/home/pi/uds"