#include "tasks/gasTask.h"
#include "sensors/mq2.h"
#include <Arduino.h>

extern MQ2Sensor gasSensor;
extern QueueHandle_t dataQueue;

void gasSensorTask(void *pvParameters)
{
    float gasData;

    while (true)
    {
        gasSensor.update();
        gasData = gasSensor.getRawValue();

        if (xQueueSend(dataQueue, &gasData, pdMS_TO_TICKS(100)) != pdPASS) {
            Serial.println("[Gas Sensor Task] Failed to send data to queue");
        }
        else {
            Serial.print("[Gas Sensor Task] Gas Concentration (PPM): ");
            Serial.println(gasData);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Run every 2 seconds
    }
}