#include "sensors/accelerometer.h"
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

bool SensorAccelerometer::begin()
{
    return accel.begin();
}

void SensorAccelerometer::update()
{
    this->accelX = accel.getX();
    this->accelY = accel.getY();
    this->accelZ = accel.getZ();
    this->accelTotal = sqrt((this->accelX * this->accelX) + (this->accelY * this->accelY) +
                            (this->accelZ * this->accelZ));
}

float SensorAccelerometer::getTotal() const
{
    return this->accelTotal;
}

float SensorAccelerometer::getX() const
{
    return this->accelX;
}

float SensorAccelerometer::getY() const
{
    return this->accelY;
}

float SensorAccelerometer::getZ() const
{
    return this->accelZ;
}

float SensorAccelerometer::getSteps() const
{
    return steps;
}
