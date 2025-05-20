#include "tasks/dhtTask.h"
#include "SensorData.h"
#include "sensors/dht22.h"
#include <Arduino.h>
#include <cmath>

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

void sendDHTData(sensor_data_t &dhtData)
{
    EventBits_t bits = xEventGroupGetBits(networkEventGroup);
    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &dhtData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[DHT Task] Failed to send data to queue");
        }
        else
        {
            Serial.println("[DHT Task] Data sent to queue successfully.");
        }
    }
}

void dhtTask(void *parameter)
{
    SensorDHT dhtSensor(DHT_PIN);
    sensor_data_t dhtData = {};
    float oldTemp = NAN;
    float oldHum = NAN;
    float newTemp = NAN;
    float newHum = NAN;

    dhtSensor.begin();

    while (true)
    {
        dhtSensor.update();
        newTemp = dhtSensor.getTemperature();
        newHum = dhtSensor.getHumdity();

        Serial.print("[DHT Task] Temperature: ");
        Serial.println(newTemp);
        Serial.print("[DHT Task] Humidity: ");
        Serial.println(newHum);

        if (isnan(oldTemp) || isnan(oldHum) || fabs(newTemp - oldTemp) > TEMP_DELTA_THRESHOLD ||
            fabs(newHum - oldHum) > HUM_DELTA_THRESHOLD)
        {
            dhtData.temperature = newTemp;
            dhtData.humidity = newHum;
            sendDHTData(dhtData);
            oldTemp = newTemp;
            oldHum = newHum;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}