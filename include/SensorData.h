#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>

typedef struct SensorData
{
    uint8_t device_battery;
    float accelZ;
    float accelTotal;
    float accelPitch;
    float accelRoll;
    bool fall_detected;
    float temperature;
    float humidity;
    float gasLevel;
    uint32_t steps;
    uint16_t heartRate;
} sensor_data_t;

typedef struct
{
    char json[256];
} processed_data_t;

#endif
