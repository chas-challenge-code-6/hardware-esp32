#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>

typedef struct SensorData
{
    uint8_t device_battery{0};
    uint8_t noise_level{0};
    float accelZ{0.0};
    float accelTotal{0.0};
    float accelPitch{0.0};
    float accelRoll{0.0};
    bool fall_detected{false};
    float temperature{0.0};
    float humidity{0.0};
    float gasLevel{0.0};
    uint32_t steps{0};
    uint16_t heartRate{0};
} sensor_data_t;

typedef struct
{
    char json[256];
} processed_data_t;

#endif
