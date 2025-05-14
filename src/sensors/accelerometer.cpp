#include "sensors/accelerometer.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Arduino.h>
#include <Wire.h>

bool SensorAccelerometer::begin()
{
    return accel.begin();
}

void SensorAccelerometer::update()
{
    accelX = accel.getX();
    accelY = accel.getY();
    accelZ = accel.getZ();
    // steps = accel.getSteps();
}

float SensorAccelerometer::getX() const
{
    return accelX;
}

float SensorAccelerometer::getY() const
{
    return accelY;
}

float SensorAccelerometer::getZ() const
{
    return accelZ;
}

float SensorAccelerometer::getSteps() const
{
    return steps;
}
