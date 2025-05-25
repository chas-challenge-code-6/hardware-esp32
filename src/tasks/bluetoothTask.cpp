#include "tasks/bluetoothTask.h"
#include "SensorData.h"
#include "config.h"
#include "network/bluetooth.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <cstring>

#define QUEUE_SEND_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

void sendBluetoothData(const sensor_message_t &msg)
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

void bluetoothTask(void *pvParameters)
{
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    BluetoothClient bClient;
    int oldHeartRate = -1;
    int newHeartRate = 0;

    bClient.begin();

    while (true)
    {
        bClient.loop(); // rewrite to use eventgroup

        newHeartRate = bClient.getHeartRate();
        if (oldHeartRate == -1 || newHeartRate != oldHeartRate)
        {
            msg.data.heartRate = newHeartRate;
            msg.valid.heartRate = 1;
            sendBluetoothData(msg);
            safePrint("[BT Task] HR: ");
            safePrintln(newHeartRate);
            oldHeartRate = newHeartRate;
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
