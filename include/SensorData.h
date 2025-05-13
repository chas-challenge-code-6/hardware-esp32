#ifndef SENSORDATA_H
#define SENSORDATA_H

typedef struct SensorData
{
    float accelX;
    float accelY;
    float accelZ;
    uint8_t device_battery;
    bool fall_detected;
    float temperature;
    float humidity;
    float gasLevel;
    uint32_t steps;
    uint8_t heartRate;
} sensor_data_t;

typedef struct
{
    char json[256];
} processed_data_t;

#endif
