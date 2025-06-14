/**
 * @file dhtTask.cpp
 *
 * @brief DHT Task Implementation File
 *
 * @details This file contains the implementation of the dhtTask function, which is used to handle
 * DHT sensor operations in a FreeRTOS task. The task is responsible for reading temperature
 * and humidity data from the DHT sensor and sending it to a queue for processing.
 *
 */

#include "tasks/dhtTask.h"
#include "SensorData.h"
#include "sensors/dht22.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <cmath>
#include <cstring>

#define QUEUE_SEND_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

// This is what the sensor can handle, anything outside this is invalid
#define DHT_MIN_TEMP -40.0f
#define DHT_MAX_TEMP 80.0f
#define DHT_MIN_HUM 0.0f
#define DHT_MAX_HUM 100.0f

bool isValidDHTReading(float temp, float hum)
{
    return (temp >= DHT_MIN_TEMP && temp <= DHT_MAX_TEMP && hum >= DHT_MIN_HUM &&
        hum <= DHT_MAX_HUM && !isnan(temp) && !isnan(hum));
}

void sendDHTData(const sensor_message_t& msg)
{
    EventBits_t bits;

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bits = xEventGroupGetBits(networkEventGroup);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("[DHT Task] Failed to access network event group");
        return;
    }

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, QUEUE_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) != pdPASS)
        {
            safePrintln("[DHT Task] Failed to send data to queue");
        }
    }
}

/**
 * @brief DHT Task function
 *
 * @details This function handles DHT sensor operations in a FreeRTOS task. It reads temperature and
 * humidity data from the DHT sensor and sends it to a queue for processing. The task runs in an
 * infinite loop, waiting for data to be available in the queue. When data is received, it is sent
 * to the network for processing.
 *
 */
void dhtTask(void* parameter)
{
    SensorDHT dhtSensor(DHT_PIN);
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    float oldTemp = NAN;
    float oldHum = NAN;
    float newTemp = NAN;
    float newHum = NAN;
    bool lastWasZero = false;

    dhtSensor.begin();
    safePrintln("[DHT Task] DHT sensor started");

    while (true)
    {
        memset(&msg, 0, sizeof(msg));

        dhtSensor.update();
        newTemp = dhtSensor.getTemperature();
        newHum = dhtSensor.getHumdity();

#if DEBUG
        safePrintf("[DHT Task] T: %.2f°C, H: %.2f%%\n", newTemp, newHum);
#endif

        if (!isValidDHTReading(newTemp, newHum))
        {
            safePrintln("[DHT Task] Invalid sensor reading, skipping...");
            vTaskDelay(pdMS_TO_TICKS(60000));
            continue;
        }

        bool currentIsZero = (newTemp == 0.0f);
        bool skipZeroReading =
            currentIsZero && !lastWasZero && (!isnan(oldTemp) && fabs(oldTemp) > 2.0f);

        bool isFirstReading = (isnan(oldTemp) || isnan(oldHum));
        bool shouldSend = isFirstReading || (fabs(newTemp - oldTemp) > TEMP_DELTA_THRESHOLD ||
            fabs(newHum - oldHum) > HUM_DELTA_THRESHOLD);

        if (!skipZeroReading && shouldSend)
        {
            if (isFirstReading)
            {
                safePrintf("[DHT Task] Sending first reading: T=%.2f°C, H=%.2f%%\n", newTemp,
                    newHum);
            }
            else
            {
                safePrintf("[DHT Task] Sending: T=%.2f°C (Δ%.2f), H=%.2f%% (Δ%.2f)\n", newTemp,
                    fabs(newTemp - oldTemp), newHum, fabs(newHum - oldHum));
            }

            msg.data.temperature = newTemp;
            msg.valid.temperature = 1;
            msg.data.humidity = newHum;
            msg.valid.humidity = 1;

            sendDHTData(msg);

            oldTemp = newTemp;
            oldHum = newHum;
        }

        lastWasZero = currentIsZero;
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}