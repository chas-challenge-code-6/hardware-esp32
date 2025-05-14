#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "main.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Arduino.h>
#include <Wire.h>

class SensorAccelerometer
{
private:
    Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
    float accelX;
    float accelY;
    float accelZ;
    int steps;

public:
    // SensorAccelerometer(uint8_t pin) : pin(pin), accelX(0), accelY(0), accelZ(0) {}

    bool begin();

    void update();

    float getX() const;
    float getY() const;
    float getZ() const;

    float getLocation() const;
    float getSteps() const;
};

#endif
