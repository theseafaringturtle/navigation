struct LSM9DS1_Message {
    char sensor;
    double reading_x;
    double reading_y;
    double reading_z;
};

#define MESSAGE_SIZE 64 //sizeof(LSM9DS1_Message) 

#define SV_SOCK_PATH "/home/pi/uds"