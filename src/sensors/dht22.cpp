#include "sensors/dht22.h"
#include <Arduino.h>
#include <DHT.h>

void SensorDHT::begin()
{
    dht.begin();
}

void SensorDHT::update()
{
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
}

float SensorDHT::getTemperature() const
{
    return temperature;
}

float SensorDHT::getHumdity() const
{
    return humidity;
}