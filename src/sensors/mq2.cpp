#include "sensors/mq2.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

MQ2Sensor::MQ2Sensor(uint8_t pin, const char* board, float voltageResolution, uint8_t adcBitResolution, const char* type)
    : mq2(board, voltageResolution, adcBitResolution, pin, type) {}

void MQ2Sensor::begin() {
    mq2.init();
}

void MQ2Sensor::update()
{
    mq2.update();
}

float MQ2Sensor::getValue()
{
    return mq2.readSensor();
}