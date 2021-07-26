
//todo change to struct since this is unlikely to change

struct LSM9DS1_Message {
    char sensor;
    double reading_x;
    double reading_y;
    double reading_z;
    int delta;
};
#define MESSAGE_SIZE sizeof(LSM9DS1_Message)  // sensor header, reading, delta time

#define SV_SOCK_PATH "/home/pi/uds"
// #define CL_SOCK_PATH "/home/pi/uds_cl" // use SIGUSR1, 2 for recalibration and shutdown?

// Set up Unix datagram socket
int setup_unix_socket(const char* path);

// Connect to Unix socket
int connect_unix(const char* path);

// todo move to main class
// Compose a message of: one char to specify sensor, three doubles for the
// dimension readings, one int for delta time
void compose_message(char* buf);

int write_restart_message();
