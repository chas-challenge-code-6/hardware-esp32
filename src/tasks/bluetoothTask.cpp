#include "tasks/bluetoothTask.h"
#include "SensorData.h"
#include "main.h"
#include "network/bluetooth.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

void sendBluetoothData(sensor_message_t msg)
{
    EventBits_t bits = xEventGroupGetBits(networkEventGroup);
    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[Bluetooth Task] Failed to send heart rate data to queue");
        }
        else
        {
            Serial.println("[Bluetooth Task] Data sent to queue successfully.");
        }
    }
}

void bluetoothTask(void *pvParameters)
{
    sensor_message_t msg = {};
    BluetoothClient bClient;
    int oldHeartRate = 0;
    int newHeartRate = 0;

    bClient.begin();

    while (true)
    {
        bClient.loop(); // rewrite to use eventgroup

        newHeartRate = bClient.getHeartRate();
        if (isnan(oldHeartRate) || newHeartRate != oldHeartRate)
        {
            msg.data.heartRate = newHeartRate;
            msg.valid.heartRate = true;
            sendBluetoothData(msg);
            oldHeartRate = newHeartRate;
            msg.valid.heartRate = false;
        }
        Serial.print("[Bluetooth Task] Heart Rate: ");
        Serial.println(newHeartRate);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
