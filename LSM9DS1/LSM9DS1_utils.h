
#include "library/LSM9DS1.h"
#include "library/LSM9DS1_Types.h"

class LSM9DS1_Sensor;

// These are bridge classes for APIs that do not accept arbitrary arguments

class LSM9DS1SendCallback : public LSM9DS1callback {
   public:
    LSM9DS1_Sensor* m_sensor;

    LSM9DS1SendCallback(LSM9DS1_Sensor* sensor);

    void hasSample(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
};

class LSM9DS1_SignalDIspatcher {
   private:
    static LSM9DS1_Sensor* _sensor;

   public:
    LSM9DS1_SignalDIspatcher(LSM9DS1_Sensor* sensor);
    static void consumer_signal(int sigcode);
};