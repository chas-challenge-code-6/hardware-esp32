#include "sensors/mq2.h"
#include "main.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

MQ2Sensor::MQ2Sensor(uint8_t pin, const char *board, float voltageResolution,
                     uint8_t adcBitResolution, const char *type)
    : mq2(board, voltageResolution, adcBitResolution, pin, type)
{
}

void MQ2Sensor::begin()
{
    mq2.init();
    mq2.setRegressionMethod(1);
    mq2.setA(GAS_SETA);
    mq2.setB(GAS_SETB);
}

void MQ2Sensor::calibrate()
{
    Serial.println("Calibrating, please wait.");
    float calcR0 = 0;
    for (size_t i = 1; i <= 10; i++)
    {
        mq2.update();
        calcR0 += mq2.calibrate(GAS_RATIO_CLEANAIR);
        Serial.print(".");
    }
    mq2.setR0(calcR0 / 10);
    Serial.println(" ...done.");
}

void MQ2Sensor::update()
{
    mq2.update();
}

int MQ2Sensor::getValue()
{
    float rawValue = mq2.readSensor();
    
    if (rawValue < 0.0f || rawValue > 10000.0f || isnan(rawValue) || isinf(rawValue)) {
        return -1;
    }
    
    int intValue = (int)round(rawValue);
    return intValue;
}