/**
 * @file bluetoothTask.cpp
 * @brief
 *
 * @details This file contains the implementation of the bluetoothTask function, which is used to
 * handle Bluetooth operations in a FreeRTOS task.
 *
 */

#include "tasks/bluetoothTask.h"
#include "SensorData.h"
#include "config.h"
#include "sensors/bluetooth.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <cstring>

#define QUEUE_SEND_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

void sendBluetoothData(const sensor_message_t& msg)
{
    EventBits_t bits;

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bits = xEventGroupGetBits(networkEventGroup);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("[BT Task] Failed to access network event group");
        return;
    }

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, QUEUE_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) != pdPASS)
        {
            safePrintln("[BT Task] Failed to send data to queue");
        }
    }
}

/**
 * @brief bluetoothTask function
 *
 * @details This function handles Bluetooth operations in a FreeRTOS task. It reads heart rate data
 * from the Bluetooth client and sends it to a queue for processing.
 *
 * @param pvParameters
 */
void bluetoothTask(void* pvParameters)
{
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    BluetoothClient bClient;
    int16_t oldHeartRate = -1;
    int16_t newHeartRate = 0;
    uint32_t lastDataTime = 0;
    const uint32_t DATA_TIMEOUT_MS = 30000;

    bClient.begin();

    while (true)
    {
        bClient.loop();

        newHeartRate = bClient.getHeartRate();

        // validate, heart rate couuuuld possibly go beyond 255, but that shit is cray cray
        if (newHeartRate > 0 && newHeartRate < 255)
        {
            lastDataTime = millis();

            if (oldHeartRate == -1 || newHeartRate != oldHeartRate)
            {
                memset(&msg, 0, sizeof(msg));
                msg.data.heartRate = newHeartRate;
                msg.valid.heartRate = 1;
                sendBluetoothData(msg);
                safePrint("[BT Task] HR: ");
                safePrintln(newHeartRate);
                oldHeartRate = newHeartRate;
            }
        }
        else if (millis() - lastDataTime > DATA_TIMEOUT_MS && lastDataTime > 0)
        {
            safePrintln("[BT Task] No valid heart rate data for 30 seconds");
            lastDataTime = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
