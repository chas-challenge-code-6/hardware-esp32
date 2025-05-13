#include "SensorData.h"
#include "tasks/processingTask.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t httpQueue;


void processingTask(void *pvParameters)
{
    sensor_data_t incomingData;

    while (true)
    {
        if (xQueueReceive(dataQueue, &incomingData, portMAX_DELAY) == pdPASS) 
        {
            Serial.println("[Processing Task] Processing data...");
            //incomingData.timestamp = millis(); // Add timestamp to the data
            
            if (xQueueSend(httpQueue, &incomingData, pdMS_TO_TICKS(100)) != pdPASS) 
            {
                Serial.println("[Processing Task] Failed to send data to HTTP queue");
            }

        }
    }
}