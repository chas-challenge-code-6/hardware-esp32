#include "tasks/bluetoothTask.h"
#include "SensorData.h"
#include "main.h"
#include "network/bluetooth.h"
#include <Arduino.h>

extern BluetoothClient bClient;
extern QueueHandle_t dataQueue;

void bluetoothTask(void *pvParameters)
{
    sensor_data_t heartRateData = {};

    while (true)
    {
        // Simulate reading heart rate data from Bluetooth
        heartRateData.heartRate = bClient.getHeartRate();

        if (xQueueSend(dataQueue, &heartRateData, pdMS_TO_TICKS(100)) != pdPASS) {
            Serial.println("[Bluetooth Task] Failed to send heart rate data to queue");
        }
        else {
            Serial.print("[Bluetooth Task] Heart Rate: ");
            Serial.println(heartRateData.heartRate);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Run every 1 second
    }
}
