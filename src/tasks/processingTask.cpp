#include "tasks/processingTask.h"
#include "SensorData.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t httpQueue;

void processingTask(void *pvParameters)
{
    sensor_data_t incomingData;
    processed_data_t processedData;

    char buffer[256];

    while (true)
    {
        if (xQueueReceive(dataQueue, &incomingData, portMAX_DELAY))
        {
            snprintf(buffer, sizeof(buffer),
                     "{\"steps\": %d, \"humidity\": %d, \"gas\": %d, \"fall_detected\": %d, "
                     "\"device_battery\": %d, \"heart_rate\": %d, \"noise_level\": %d }",
                     incomingData.steps, incomingData.temperature, incomingData.humidity,
                     incomingData.fall_detected, incomingData.device_battery);
        }
        strcpy(processedData.json, buffer);

        xQueueSend(httpQueue, &processedData, portMAX_DELAY);
    }
}