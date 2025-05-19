/**
 * @file processingTask.cpp
 * @brief Processing Task Implementation File
 * 
 * @details This file contains the implementation of the processingTask function, which is used to handle
 * processing operations in a FreeRTOS task. The task is responsible for processing sensor data
 * and creating a JSON string for network transmission.
 * 
 */

#include "tasks/processingTask.h"
#include "SensorData.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue; 
extern QueueHandle_t httpQueue; 


/**
 * @brief Create a Json object
 * 
 * @details This function creates a JSON string from the sensor data and stores it in the provided buffer.
 * The JSON string contains the following fields:
 * - steps
 * - humidity
 * - gas
 * - fall_detected
 * - device_battery
 * - heart_rate
 * - noise_level
 * 
 * @param data 
 * @param buffer 
 * @param bufferSize 
 * @return true 
 * @return false 
 */

bool createJson(const sensor_data_t &data, char *buffer, size_t bufferSize)
{
    int len = snprintf(buffer, sizeof(bufferSize),
                       "{\"steps\": %d, \"humidity\": %d, \"gas\": %d, \"fall_detected\": %d, "
                       "\"device_battery\": %d, \"heart_rate\": %d, \"noise_level\": %d }",
                       data.steps, data.temperature, data.humidity, data.fall_detected,
                       data.device_battery);
    if (len < 0 || len > bufferSize)
    {
        Serial.println("[Processing Task] JSON creation failed or truncated.");
        return false;
    }
    return true;
}



/**
 * @brief Processing Task function
 * 
 * @details This function handles processing operations in a FreeRTOS task. It reads sensor data from a queue,
 * processes it, and creates a JSON string for network transmission. The task runs in an infinite loop,
 * waiting for data to be available in the queue. When data is received, it is processed and sent to
 * the HTTP queue for transmission.
 * 
 * @param pvParameters 
 */

void processingTask(void *pvParameters)
{
    sensor_data_t incomingData;
    processed_data_t processedData;
    char buffer[256];

    while (true)
    {
        if (xQueueReceive(dataQueue, &incomingData, portMAX_DELAY))
        {
            if (createJson(incomingData, buffer, sizeof(buffer)))
            {
                strncpy(processedData.json, buffer, sizeof(processedData.json) - 1);
                processedData.json[sizeof(processedData.json) - 1] = '\0';

                if (xQueueSend(httpQueue, &processedData, 2000 / portTICK_PERIOD_MS) != pdPASS)
                {
                    Serial.println("[Processing Task] Failed to send JSON to HTTP queue.");
                }
                else
                {
                    Serial.print("[Processing Task] Failed to send JSON to HTTP queue.");
                    Serial.println(processedData.json);
                }
            }
        }
    }
}