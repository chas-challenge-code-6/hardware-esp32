/**
 * @file main.cpp
 * @brief Main File
 *
 * @details This file contains the main function and setup function for the ESP32 project. It
 * initializes the necessary components, creates FreeRTOS tasks, and sets up the event group for
 * network communication.
 *
 *
 */

#include "SensorData.h"
#include "config.h"
#include "tasks/accelerometerTask.h"
#include "tasks/batteryTask.h"
#include "tasks/bluetoothTask.h"
#include "tasks/communicationTask.h"
#include "tasks/dhtTask.h"
#include "tasks/gasTask.h"
#include "tasks/networkStatusTask.h"
#include "tasks/processingTask.h"
#include "tasks/GPStask.h"
#include "utils/threadsafe_serial.h"
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

    // Critical priority tasks (fall detection)
    xTaskCreatePinnedToCore(accelTask, "AccelTask", 8192, NULL, 4, NULL, 1);

    // High priority tasks
    xTaskCreatePinnedToCore(processingTask, "ProcessTask", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(communicationTask, "CommTask", 8192, NULL, 3, NULL, 1);
    

    // Medium priority tasks
    xTaskCreate(gasTask, "Gas Task", 4096, NULL, 2, NULL);
    xTaskCreatePinnedToCore(networkStatusTask, "networkStatusTask", 8192, NULL, 2, NULL, 1);
    //xTaskCreate(bluetoothTask, "Bluetooth Task", 8192, NULL, 2, NULL);
    xTaskCreate(dhtTask, "DHT Task", 8192, NULL, 2, NULL);

    // Low priority tasks
    //xTaskCreate(dhtTask, "DHT Task", 4096, NULL, 2, NULL);
    xTaskCreate(batteryTask, "Battery Task", 4096, NULL, 1, NULL);
    xTaskCreate(gpsTask, "GPSTask", 8192, NULL, 1, NULL);
    

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        xEventGroupSetBits(networkEventGroup, SYSTEM_READY_BIT);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("Failed to initialize system ready bit!");
        while (1)
            ;
    }

    ("Sentinel started.");
}

/**
 * @brief Main loop
 *
 * @details The main loop is empty as all tasks are handled in FreeRTOS tasks.
 * The loop function is required by the Arduino framework, but it does not perform any operations.
 */
void loop() {}
