#include "tasks/dhtTask.h"
#include "SensorData.h"
#include "sensors/dht22.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;

void dhtTask(void *parameter)
{
    SensorDHT dhtSensor(DHT_PIN);
    sensor_data_t accData = {};

    dhtSensor.begin();

    while (true)
    {
        dhtSensor.update();
        accData.temperature = dhtSensor.getTemperature();
        accData.humidity = dhtSensor.getHumdity();

        Serial.print("[DHT Task] Temperature: ");
        Serial.println(accData.temperature);
        Serial.print("[DHT Task] Humidity: ");
        Serial.println(accData.humidity);

        if(xQueueSend(dataQueue, &accData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[DHT Task] Failed to send data to queue");
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}