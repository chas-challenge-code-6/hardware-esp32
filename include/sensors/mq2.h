/**
 * @file mq2.h
 * @brief MQ-2 Gas Sensor
 * @date 2025-05-19
 * 
 * File to handle the MQ-2 gas sensor class.
 * 
 */

#ifndef SENSOR_MQ2
#define SENSOR_MQ2

#include "config.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

/**
 * @brief Class for the MQ-2 gas sensor
 * 
 * 
 * 
 */


class MQ2Sensor
{
public:
    MQ2Sensor(uint8_t pin, const char *board = "ESP-32", float voltageResolution = 3.3,
              uint8_t adcBitResolution = 12, const char *type = "MQ-2");
    void update();
    float getValue();
    void begin();
    void calibrate();

private:
    MQUnifiedsensor mq2;
};

#endif