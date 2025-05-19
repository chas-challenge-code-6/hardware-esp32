#include "tasks/bluetoothTask.h"
#include "SensorData.h"
#include "main.h"
#include "network/bluetooth.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

void sendBluetoothData(sensor_data_t &heartRateData)
{
    EventBits_t bits = xEventGroupGetBits(networkEventGroup);
    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &heartRateData, portMAX_DELAY) != pdPASS)
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
    sensor_data_t heartRateData = {};
    BluetoothClient bClient;
    float oldHeartRate = NAN;
    float newHeartRate = NAN;

    while (true)
    {
        newHeartRate = bClient.getHeartRate();
        if (isnan(oldHeartRate) || newHeartRate != oldHeartRate)
        {
            heartRateData.heartRate = newHeartRate;
            sendBluetoothData(heartRateData);
            oldHeartRate = newHeartRate;
        }
        Serial.print("[Bluetooth Task] Heart Rate: ");
        Serial.println(newHeartRate);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
