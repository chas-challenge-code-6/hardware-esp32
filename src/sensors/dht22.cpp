/**
 * @file dht22.cpp
 * @brief DHT22 Sensor
 * @details File to handle the DHT22 class
 * @date 2025-05-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "sensors/dht22.h"
#include <Arduino.h>
#include <DHT.h>

/**
 * @brief Constructor for the SensorDHT class
 *
 * @param pin Pin number where the DHT22 sensor is connected
 */
void SensorDHT::begin()
{
    dht.begin();
}

/**
 * @brief Update the temperature and humidity values
 *
 * This function reads the temperature and humidity from the DHT22 sensor
 * and updates the corresponding member variables.
 */
void SensorDHT::update()
{
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
}

/**
 * @brief Get the temperature value
 *
 * @return float The temperature value in Celsius
 */
float SensorDHT::getTemperature() const
{
    return temperature;
}

/**
 * @brief Get the humidity value
 *
 * @return float The humidity value in percentage
 */
float SensorDHT::getHumdity() const
{
    return humidity;
}