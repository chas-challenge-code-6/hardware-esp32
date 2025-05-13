#include "sensors/mq2.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

MQ2Sensor::MQ2Sensor(uint8_t pin, const String& board, float voltageResolution, int adcBitResolution, const String& type)
    : mq2(board, voltageResolution, adcBitResolution, pin, type), _pin(pin), _lastRaw(0) {}

void MQ2Sensor::begin() {
    mq2.init();
}

void MQ2Sensor::update()
{
    _lastRaw = analogRead(_pin);  
}

float MQ2Sensor::getRawValue()
{
    return (float)_lastRaw;  
}