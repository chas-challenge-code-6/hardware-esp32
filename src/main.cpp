#include "main.h"
#include "SensorData.h"
#include "network/bluetooth.h"
#include "tasks/accelerometerTask.h"
#include "tasks/bluetoothTask.h"
#include "tasks/communicationTask.h"
#include "tasks/dhtTask.h"
#include "tasks/gasTask.h"
#include "tasks/processingTask.h"

#include <Arduino.h>
#include <Wire.h>

// TODO: move these into tasks
BluetoothClient bClient;

QueueHandle_t dataQueue;
QueueHandle_t httpQueue;

EventGroupHandle_t networkEventGroup;

void setup()
{
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);

    networkEventGroup = xEventGroupCreate();

    dataQueue = xQueueCreate(10, sizeof(sensor_data_t));
    httpQueue = xQueueCreate(10, sizeof(sensor_data_t));

    xTaskCreatePinnedToCore(accelTask, "AccelTask", 2048, NULL, 1, NULL, 1); // Pin to core 1 to not disturb WiFi/LTE
    // xTaskCreate(bluetoothTask, "Bluetooth Task", 2048, NULL, 1, NULL);
    //xTaskCreate(dhtTask, "DHT Task", 2048, NULL, 1, NULL);
    //xTaskCreate(gasTask, "Gas Task", 2048, NULL, 1, NULL);
    // xTaskCreate(communicationTask, "CommTask", 4096, &comm, 1, NULL);
    //xTaskCreate(processingTask, "Process", 4096, NULL, 1, NULL);
}

void loop() {}