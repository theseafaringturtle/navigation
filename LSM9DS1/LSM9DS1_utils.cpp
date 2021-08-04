#include "LSM9DS1_sensor.h"
#include "library/LSM9DS1.h"
#include "LSM9DS1_utils.h"

void LSM9DS1SendCallback::set_sensor(LSM9DS1_Sensor *sensor)
{
    this->m_sensor = sensor;
}

void LSM9DS1SendCallback::hasSample(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz)
{
    this->m_sensor->has_sample(gx, gy, gz, ax, ay, az, mx, my, mz);
}


LSM9DS1_Sensor* LSM9DS1_SignalDIspatcher::_sensor;
void LSM9DS1_SignalDIspatcher::set_sensor(LSM9DS1_Sensor *sensor)
{
    LSM9DS1_SignalDIspatcher::_sensor = sensor;
}
void LSM9DS1_SignalDIspatcher::consumer_signal(int sigcode)
{
    LSM9DS1_SignalDIspatcher::_sensor->consumer_signal(sigcode);
}
