#include "sensors/accelerometer.h"
#include <Arduino.h>
//#include <Accelerometer.h>

void SensorAccelerometer::begin()
{
    //accel.begin();
}

void SensorAccelerometer::update()
{
    /* accelX = accelX.getX();
    accelY = accelY.getY();
    accelZ = accelZ.getZ(); */
}

float SensorAccelerometer::getLocation() const
{
    return accelX, accelY, accelZ;
}

float SensorAccelerometer::getSteps() const
{
    //return steps;
}