#ifndef SENSOR_MQ2
#define SENSOR_MQ2

#include <MQUnifiedsensor.h>
#include <Arduino.h>
#include "main.h"

class MQ2Sensor {
public:
    MQ2Sensor(uint8_t pin, const String& board = "ESP-32", float voltageResolution = 3.3, int adcBitResolution = 12, const String& type = "MQ-2");
    void update();         
    float getPPM(); 

private:
    MQUnifiedsensor mq2;
    uint8_t _pin;
};

#endif