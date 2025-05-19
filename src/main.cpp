/**
 * @file main.cpp
 * @brief Main File
 * 
 * @details This file contains the main function and setup function for the ESP32 project. It initializes
 * the necessary components, creates FreeRTOS tasks, and sets up the event group for network communication.
 * 
 * 
 */

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

QueueHandle_t dataQueue; ///< Queue for sensor data, used by all tasks
QueueHandle_t httpQueue; ///< Queue for HTTP data, used by the communication task

EventGroupHandle_t networkEventGroup; ///< Event group for network events

void setup()
{
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN, 100000); ///< Initialize I2C

    networkEventGroup = xEventGroupCreate(); ///< Event group for network events

    dataQueue = xQueueCreate(10, sizeof(sensor_data_t)); ///< Queue for sensor data, used by all tasks
    httpQueue = xQueueCreate(10, sizeof(sensor_data_t)); ///< Queue for HTTP data, used by the communication task

    xTaskCreatePinnedToCore(accelTask, "AccelTask", 4096, NULL, 1, NULL, 
                            1); // Pin to core 1 to not disturb WiFi/LTE ///< Task for accelerometer
    // xTaskCreate(bluetoothTask, "Bluetooth Task", 2048, NULL, 1, NULL); ///< Task for Bluetooth
    // xTaskCreate(dhtTask, "DHT Task", 2048, NULL, 1, NULL); ///< Task for DHT sensor
    // xTaskCreate(gasTask, "Gas Task", 2048, NULL, 1, NULL); ///< Task for gas sensor
    // xTaskCreate(communicationTask, "CommTask", 4096, &comm, 1, NULL); ///< Task for communication
    // xTaskCreate(processingTask, "Process", 4096, NULL, 1, NULL); ///< Task for processing data
}


/** 
 * @brief Main loop
 * 
 * @details The main loop is empty as all tasks are handled in FreeRTOS tasks.
 * The loop function is required by the Arduino framework, but it does not perform any operations.
 */
void loop() {}