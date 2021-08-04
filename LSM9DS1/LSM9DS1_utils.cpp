#include "LSM9DS1_sensor.h"
#include "library/LSM9DS1.h"

class LSM9DS1SendCallback : public LSM9DS1callback {
   public:
    LSM9DS1_Sensor* m_sensor;

    LSM9DS1SendCallback(LSM9DS1_Sensor* sensor) {
        m_sensor = sensor;
    }

    void hasSample(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
        m_sensor->has_sample(gx, gy, gz, ax, ay, az, mx, my, mz);
    }
};

class LSM9DS1_SignalDIspatcher {
   private:
    static LSM9DS1_Sensor* _sensor;

   public:
    LSM9DS1_SignalDIspatcher(LSM9DS1_Sensor* sensor) {
        _sensor = sensor;
    }
    static void consumer_signal(int sigcode) {
        _sensor->consumer_signal(sigcode);
    }
};
