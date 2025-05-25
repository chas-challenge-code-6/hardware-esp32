#include "main.h"
#include "SensorData.h"
#include "network/bluetooth.h"
#include "tasks/accelerometerTask.h"
#include "tasks/bluetoothTask.h"
#include "tasks/communicationTask.h"
#include "tasks/dhtTask.h"
#include "tasks/gasTask.h"
#include "tasks/networkStatusTask.h"
#include "tasks/processingTask.h"
#include "utilities.h"
#include <TinyGsmClient.h>
#include <WiFi.h>

#include <Arduino.h>
#include <Wire.h>

QueueHandle_t dataQueue;
QueueHandle_t httpQueue;

EventGroupHandle_t networkEventGroup;
#define SYSTEM_READY_BIT BIT1

TinyGsm modem(SerialAT);

void setup()
{
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN, 100000);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);

    networkEventGroup = xEventGroupCreate();

    dataQueue = xQueueCreate(10, sizeof(sensor_data_t));
    httpQueue = xQueueCreate(10, sizeof(sensor_data_t));

    xTaskCreatePinnedToCore(accelTask, "AccelTask", 4096, NULL, 1, NULL, 1);
    //xTaskCreate(bluetoothTask, "Bluetooth Task", 4096, NULL, 1, NULL);
    xTaskCreate(dhtTask, "DHT Task", 2048, NULL, 1, NULL);
    xTaskCreate(gasTask, "Gas Task", 4096, NULL, 1, NULL);
    xTaskCreatePinnedToCore(communicationTask, "CommTask", 6144, NULL, 1, NULL,
                            ARDUINO_RUNNING_CORE);
    xTaskCreatePinnedToCore(networkStatusTask, "networkStatusTask", 2096, NULL, 1, NULL,
                            ARDUINO_RUNNING_CORE);
    xTaskCreate(processingTask, "Process", 4096, NULL, 1, NULL);

    xEventGroupSetBits(networkEventGroup, SYSTEM_READY_BIT);

    Serial.println("Sentinel started.");
}

void loop() {}