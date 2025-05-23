#ifndef SENSORS_DHT22
#define SENSORS_DHT22

#include "main.h"
#include <Arduino.h>
#include <DHT.h>

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
