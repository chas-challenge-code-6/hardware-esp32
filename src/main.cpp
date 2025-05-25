#include "SensorData.h"
#include "config.h"
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

SemaphoreHandle_t serialMutex;
SemaphoreHandle_t modemMutex;
SemaphoreHandle_t networkEventMutex;

TinyGsm modem(SerialAT);

void setup()
{
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN, 100000);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);

    serialMutex = xSemaphoreCreateMutex();
    modemMutex = xSemaphoreCreateMutex();
    networkEventMutex = xSemaphoreCreateMutex();

    if (serialMutex == NULL || modemMutex == NULL || networkEventMutex == NULL)
    {
        Serial.println("Failed to create mutexes!");
        while (1)
            ;
    }

    networkEventGroup = xEventGroupCreate();

    dataQueue = xQueueCreate(10, sizeof(sensor_message_t));
    httpQueue = xQueueCreate(10, sizeof(processed_data_t));

    xTaskCreatePinnedToCore(accelTask, "AccelTask", 8192, NULL, 1, NULL, 1);
    xTaskCreate(bluetoothTask, "Bluetooth Task", 4096, NULL, 1, NULL);
    xTaskCreate(dhtTask, "DHT Task", 4096, NULL, 1, NULL);
    xTaskCreate(gasTask, "Gas Task", 4096, NULL, 1, NULL);
    xTaskCreatePinnedToCore(communicationTask, "CommTask", 8192, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(networkStatusTask, "networkStatusTask", 8192, NULL, 1, NULL, 1);
    xTaskCreate(processingTask, "Process", 4096, NULL, 1, NULL);

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        xEventGroupSetBits(networkEventGroup, SYSTEM_READY_BIT);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        Serial.println("Failed to initialize system ready bit!");
        while (1)
            ;
    }

    Serial.println("Sentinel started.");
}

void loop() {}