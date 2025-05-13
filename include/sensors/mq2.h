#ifndef SENSOR_MQ2
#define SENSOR_MQ2

#include "main.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

class MQ2Sensor
{
public:
    MQ2Sensor(uint8_t pin, const char *board = "ESP-32", float voltageResolution = 3.3,
              uint8_t adcBitResolution = 12, const char *type = "MQ-2");
    void update();
    float getValue();
    void begin();

private:
    MQUnifiedsensor mq2;
};

#endif