#ifndef SENSORDATA_H
#define SENSORDATA_H

typedef struct SensorData
{
    float accelX;
    float accelY;
    float accelZ;
    float temperature;
    float humidity;
    float gasLevel;
    int heartRate;
} sensor_data_t;

#endif
