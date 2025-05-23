/**
 * @file mq2.cpp
 * @brief MQ-2 Gas Sensor
 * @date 2025-05-19
 * 
 * File to handle the MQ-2 gas sensor class. 
 */

#include "sensors/mq2.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

/**
 * @brief Construct a new MQ2Sensor object
 *
 * @param pin Pin number for the sensor
 * @param board Board type (default is "ESP-32")
 * @param voltageResolution Voltage resolution (default is 3.3)
 * @param adcBitResolution ADC bit resolution (default is 12)
 * @param type Sensor type (default is "MQ-2") 
 */

MQ2Sensor::MQ2Sensor(uint8_t pin, const char* board, float voltageResolution, uint8_t adcBitResolution, const char* type)
    : mq2(board, voltageResolution, adcBitResolution, pin, type) {}


/**
 * @brief Initializes the MQ2 sensor
 * 
 * @details Initializes the MQ2 sensor with the given parameters.
 * 
 */
void MQ2Sensor::begin() {
    mq2.init();
}


/**
 * @brief Updates the MQ2 sensor
 * 
 * @details Updates the MQ2 sensor with the latest readings.
 * 
 */
void MQ2Sensor::update()
{
    mq2.update();
}


/**
 * @brief Gets the value from the MQ2 sensor
 * 
 * @details Returns the value from the MQ2 sensor.
 * 
 */
float MQ2Sensor::getValue()
{
    return mq2.readSensor();
}