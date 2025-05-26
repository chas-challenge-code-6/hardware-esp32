/**
 * @file accelerometer.cpp
 * @brief 
 * @date 2025-05-19
 * 
 * File to handle the MPU6500 accelerometer class.
 * 
 */

#include "sensors/accelerometer.h"
#include <Arduino.h>
#include <MPU6500_WE.h>
#include <Wire.h>
#include <math.h>


/**
 * @brief Initializes the MPU6500 sensor
 * 
 * @return true 
 * @return false (if initialization fails)
 */
bool SensorAccelerometer::begin()
{
    if (!accel.init())
    {
        Serial.println("[Accelerometer] Failed to initialize MPU6500!");
        return false;
    }
    setup();
    return true;
}


/**
 * @brief Initializes the MPU6500 sensor with default settings
 * 
 * 
 */
void SensorAccelerometer::setup()
{
    accel.enableGyrDLPF();
    accel.setGyrDLPF(MPU6500_DLPF_6);
    accel.setSampleRateDivider(5);
    accel.setGyrRange(MPU6500_GYRO_RANGE_250);
    accel.setAccRange(MPU6500_ACC_RANGE_16G);
    accel.enableAccDLPF(true);
    accel.setAccDLPF(MPU6500_DLPF_6);
}


/**
 * @brief Updates the accelerometer values
 * 
 * @details Updates the accelerometer values and calculates pitch, roll, and total acceleration.
 * 
 */
void SensorAccelerometer::update()
{
    values = accel.getGValues();

    this->accelZ = values.z;
    this->accelTotal = sqrt((values.x * values.x) + (values.y * values.y) + (values.z * values.z));
    this->accelPitch =
        atan2(values.y, sqrt(values.x * values.x + values.z * values.z)) * 180.0 / PI;
    this->accelRoll = atan2(-values.x, values.z) * 180.0 / PI;
}

float SensorAccelerometer::getZ() const
{
    return this->accelZ;
}
float SensorAccelerometer::getTotal() const
{
    return this->accelTotal;
}

float SensorAccelerometer::getPitch() const
{
    return this->accelPitch;
}

float SensorAccelerometer::getRoll() const
{
    return this->accelRoll;
}
