#include "tasks/processingTask.h"
#include "SensorData.h"
#include "main.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t httpQueue;

bool createJson(const sensor_data_t &data, char *buffer, size_t bufferSize)
{
    int len = snprintf(buffer, bufferSize,
                       "{\"device_id\": \"%s\" \"sensors\": { \"steps\": %d, \"humidity\": %.2f, "
                       "\"gas\": \"ppm\": %d, \"fall_detected\": %d, "
                       "\"device_battery\": %d, \"heart_rate\": %d, \"noise_level\": %d } }",
                       DEVICE_ID, data.steps, data.humidity, data.gasLevel, data.fall_detected,
                       data.device_battery, data.heartRate, data.noise_level);
    if (len < 0 || len >= bufferSize)
    {
        Serial.println("[Processing Task] JSON creation failed or truncated.");
        return false;
    }
    return true;
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
                    Serial.print("[Processing Task] Sent JSON to HTTP queue.");
                    Serial.println(processedData.json);
                }
            }
        }
    }
}
