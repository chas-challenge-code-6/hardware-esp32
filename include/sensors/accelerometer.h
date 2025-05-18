#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Arduino.h>
#include <MPU6500_WE.h>
#include <Wire.h>
#include <main.h>

class SensorAccelerometer
{
private:
    MPU6500_WE accel = MPU6500_WE();
    xyzFloat values;
    int steps;
    float accelZ;
    float accelTotal;
    float accelPitch;
    float accelRoll;

public:
    bool begin();
    void setup();
    void update();
    float getZ() const;
    float getTotal() const;
    float getPitch() const;
    float getRoll() const;
    float getSteps() const;
};

#endif
