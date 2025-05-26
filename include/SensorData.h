/**
 * @file SensorData.h
 * @brief Sensor Data Header File 
 * 
 */

#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <stdint.h>


/** 
 * @brief Sensor Data Structure
 * 
 * @details This structure contains the data collected from various sensors.
 * It includes information such as device battery level, accelerometer data, temperature, humidity,
 * gas level, steps, and heart rate.
 * 
 */
typedef struct SensorData
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
    uint8_t device_battery;
    uint8_t noise_level;
    uint8_t accelZ;
    uint8_t accelTotal;
    uint8_t accelPitch;
    uint8_t accelRoll;
    uint8_t fall_detected;
    uint8_t temperature;
    uint8_t humidity;
    uint8_t gasLevel;
    uint8_t steps;
    uint8_t heartRate;
} sensor_data_flags_t;

typedef struct
{
    sensor_data_t data;
    sensor_data_flags_t valid;
} sensor_message_t;

/**
 * @brief Processed Data Structure
 * 
 * @details This structure contains the processed data in JSON format.
 * 
 */
typedef struct
{
    char json[512];
} processed_data_t;

#endif
