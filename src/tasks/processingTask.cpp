#include "tasks/processingTask.h"
#include "SensorData.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t httpQueue;

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
}

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