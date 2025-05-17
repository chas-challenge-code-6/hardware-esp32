#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>
#include <main.h>

class SensorAccelerometer
{
private:
    Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
    int steps;
    float accelX;
    float accelY;
    float accelZ;
    float accelTotal;

public:
    // SensorAccelerometer(uint8_t pin) : pin(pin), accelX(0), accelY(0), accelZ(0) {}

    bool begin();
    void update();
    float getTotal() const;
    float getX() const;
    float getY() const;
    float getZ() const;
    float getSteps() const;
};

#endif
