#include "tasks/dhtTask.h"
#include "SensorData.h"
#include "sensors/dht22.h"
#include <Arduino.h>

extern SensorDHT dhtSensor;
extern QueueHandle_t dataQueue;

void dhtTask(void *parameter)
{
    sensor_data_t accData = {};

    while (true)
    {
        dhtSensor.update();
        accData.temperature = dhtSensor.getTemperature();
        xQueueSend(dataQueue, &accData, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}