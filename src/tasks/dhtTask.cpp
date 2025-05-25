#include "tasks/dhtTask.h"
#include "SensorData.h"
#include "sensors/dht22.h"
#include <Arduino.h>
#include <cmath>

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

void sendDHTData(sensor_message_t msg)
{
    EventBits_t bits = xEventGroupGetBits(networkEventGroup);
    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, portMAX_DELAY) != pdPASS)
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
    sensor_message_t msg = {};
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

        // Skicka inte data om ej inom threshold eller om temp/humid plötsligt sjunker till 0 (om
        // innan +-2 grader) om det inte fortsätter repetera.
        static bool lastWasZeroTemp = false;
        bool suddenZeroTemp = (oldTemp > 2.0f || oldTemp < -2.0f) && newTemp == 0.0f;

        if (suddenZeroTemp && !lastWasZeroTemp)
        {
            lastWasZeroTemp = true;
        }
        else
        {
            if (isnan(oldTemp) || isnan(oldHum) || fabs(newTemp - oldTemp) > TEMP_DELTA_THRESHOLD ||
                fabs(newHum - oldHum) > HUM_DELTA_THRESHOLD)
            {
                msg.data.temperature = newTemp;
                msg.valid.temperature = true;
                msg.data.humidity = newHum;
                msg.valid.humidity = true;
                sendDHTData(msg);
                oldTemp = newTemp;
                oldHum = newHum;
                msg.valid.temperature = false;
                msg.valid.humidity = false;
            }
            lastWasZeroTemp = (newTemp == 0.0f);
        }
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}