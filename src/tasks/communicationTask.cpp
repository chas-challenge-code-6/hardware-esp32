#include "SensorData.h"
#include "network/network.h"
#include <Arduino.h>

extern QueueHandle_t httpQueue;

void communicationTask(void *pvParameters)
{
    processed_data_t outgoingData;

    while (true)
    {
        if(xQueueReceive(httpQueue, &outgoingData, portMAX_DELAY)) {
            
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
