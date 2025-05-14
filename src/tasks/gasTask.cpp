#include "tasks/gasTask.h"
#include "SensorData.h"
#include "sensors/mq2.h"
#include <Arduino.h>

extern MQ2Sensor gasSensor;
extern QueueHandle_t dataQueue;

void gasSensorTask(void *parameter)
{
    sensor_data_t gasData = {};

    while (true)
    {
        gasSensor.update();
        Serial.print("Gas Concentration (PPM): ");
        Serial.println(gasSensor.getValue());

        gasData.gasLevel = gasSensor.getValue();

        xQueueSend(dataQueue, &gasData, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}