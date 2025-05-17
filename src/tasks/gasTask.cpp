#include "tasks/gasTask.h"
#include "SensorData.h"
#include "sensors/mq2.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;

void gasTask(void *parameter)
{
    sensor_data_t gasData = {};
    MQ2Sensor gasSensor(GAS_PIN);

    while (true)
    {
        gasSensor.update();

        gasData.gasLevel = gasSensor.getValue();

        Serial.print("[Gas Task] Gas Concentration (PPM): ");
        Serial.println(gasData.gasLevel);

        if(xQueueSend(dataQueue, &gasData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[Gas Task] Failed to send data to queue");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}