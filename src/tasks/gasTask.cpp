/**
 * @file gasTask.cpp
 * @brief Gas Task Implementation File
 * 
 * @details This file contains the implementation of the gasTask function, which is used to handle
 * gas sensor operations in a FreeRTOS task. The task is responsible for reading gas concentration
 * data from the MQ2 sensor and sending it to a queue for processing.
 * 
 */

#include "tasks/gasTask.h"
#include "SensorData.h"
#include "sensors/mq2.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <cmath>
#include <cstring>


#define QUEUE_SEND_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0


/**
 * @brief Send Gas Data
 * 
 * @param msg 
 */

void sendGasData(const sensor_message_t &msg)
{
    EventBits_t bits;

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bits = xEventGroupGetBits(networkEventGroup);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("[Gas Task] Failed to access network event group");
        return;
    }

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, QUEUE_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) != pdPASS)
        {
            safePrintln("[Gas Task] Failed to send data to queue");
        }
    }
}


/**
 * @brief Gas Task function
 * 
 * @details This function handles gas sensor operations in a FreeRTOS task. It reads gas concentration
 * data from the MQ2 sensor and sends it to a queue for processing. The task runs in an infinite loop,
 * waiting for data to be available in the queue. When data is received, it is sent to the network for processing.
 * The task uses the MQ2 sensor to measure gas concentration in parts per million (PPM) and sends the data to a queue.
 * 
 * @param parameter 
 */
void gasTask(void *parameter)
{
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    MQ2Sensor gasSensor(GAS_PIN);
    int oldGasPPM = -1;
    int newGasLevel = 0;

    gasSensor.begin();
    gasSensor.calibrate();

    while (true)
    {
        memset(&msg, 0, sizeof(msg));

        gasSensor.update();
        newGasLevel = gasSensor.getValue();

        if (newGasLevel == -1)
        {
            safePrintln("[Gas Task] Sensor returned invalid reading, skipping...");
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        safePrintf("[Gas Task] Gas: %d PPM\n", newGasLevel);

        bool isFirstReading = (oldGasPPM == -1);
        bool deltaExceeded = (abs(newGasLevel - oldGasPPM) >= (int)GAS_DELTA_THRESHOLD);
        bool highGasAlert = (newGasLevel > 200);
        bool shouldSend = isFirstReading || deltaExceeded || highGasAlert;

        if (shouldSend)
        {
            if (isFirstReading)
            {
                safePrintf("[Gas Task] Sending first reading: %d PPM\n", newGasLevel);
            }
            else if (highGasAlert)
            {
                safePrintf("[Gas Task] HIGH GAS ALERT! Sending immediately: %d PPM\n", newGasLevel);
            }
            else
            {
                safePrintf("[Gas Task] Sending: %d PPM (Δ%d)\n", newGasLevel,
                           abs(newGasLevel - oldGasPPM));
            }
            msg.data.gasLevel = newGasLevel;
            msg.valid.gasLevel = 1;
            sendGasData(msg);
            oldGasPPM = newGasLevel;
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}