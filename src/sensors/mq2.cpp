#include "sensors/mq2.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

MQ2Sensor::MQ2Sensor(uint8_t pin, const String& board, float voltageResolution, int adcBitResolution, const String& type)
    : mq2(board, voltageResolution, adcBitResolution, pin, type), _pin(pin) {}

void MQ2Sensor::begin() {
    mq2.init();
}

int MQ2Sensor::readRaw() {
    mq2.update();
    return analogRead(_pin);
}