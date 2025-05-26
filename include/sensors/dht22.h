/**
 * @file dht22.h
 * @brief DHT22 Sensor
 * @date 2025-05-19
 * 
 * @details File to handle the DHT22 class
 * 
 */

#ifndef SENSORS_DHT22
#define SENSORS_DHT22

#include "config.h"
#include <Arduino.h>
#include <DHT.h>


/**
 * @class SensorDHT
 * @brief Class for the DHT22 sensor
 * @details Decalration of the DHT22 class
 * 
 * 
 */

class SensorDHT
{
private:
    DHT dht;
    float temperature;
    float humidity;

public:
    SensorDHT(uint8_t pin) : dht(pin, DHT22) {}

    void begin();
    void update();

    float getTemperature() const;
    float getHumdity() const;
};

#endif
