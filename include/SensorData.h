#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <stdbool.h>

typedef struct
{
    int device_battery;
    int noise_level;
    float accelZ;
    float accelTotal;
    float accelPitch;
    float accelRoll;
    bool fall_detected;
    float temperature;
    float humidity;
    int gasLevel;
    int steps;
    int heartRate;
} sensor_data_t;

typedef struct
{
    bool device_battery;
    bool noise_level;
    bool accelZ;
    bool accelTotal;
    bool accelPitch;
    bool accelRoll;
    bool fall_detected;
    bool temperature;
    bool humidity;
    bool gasLevel;
    bool steps;
    bool heartRate;
} sensor_data_flags_t;

typedef struct
{
    sensor_data_t data;
    sensor_data_flags_t valid;
} sensor_message_t;

typedef struct
{
    char json[256];
} processed_data_t;

#endif
