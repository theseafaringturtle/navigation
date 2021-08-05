struct Encoders_Message {
    char sensor;
    unsigned long timestamp;
    int left_ticks;
    int right_ticks;
};

#define ENCODERS_MESSAGE_SIZE sizeof(Encoders_Message) 

#define E_SOCK_PATH "/home/pi/uds_e"