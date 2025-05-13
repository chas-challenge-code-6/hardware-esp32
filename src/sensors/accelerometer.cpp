#include "sensors/accelerometer.h"
#include <Arduino.h>
#include <Adafruit_ADXL345_U.h>


void SensorAccelerometer::begin()
{
    accel.begin();
}

void SensorAccelerometer::update()
{
    accelX = accel.getX();
    accelY = accel.getY();
    accelZ = accel.getZ();
    //steps = accel.getSteps();
}

float SensorAccelerometer::getLocation() const
{
    return accelX, accelY, accelZ;
}

float SensorAccelerometer::getSteps() const
{
    return steps;
}