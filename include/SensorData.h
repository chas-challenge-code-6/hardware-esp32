/**
 * @file SensorData.h
 * @brief Sensor Data Header File 
 * 
 */

#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>


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


/**
 * @brief Processed Data Structure
 * 
 * @details This structure contains the processed data in JSON format.
 * 
 */
typedef struct
{
    char json[256];
} processed_data_t;

#endif
