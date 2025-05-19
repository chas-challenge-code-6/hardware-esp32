/**
 * @file dhtTask.cpp
 * 
 * @brief DHT Task Implementation File
 * 
 * @details This file contains the implementation of the dhtTask function, which is used to handle
 * DHT sensor operations in a FreeRTOS task. The task is responsible for reading temperature
 * and humidity data from the DHT sensor and sending it to a queue for processing.
 * 
 */


#include "tasks/dhtTask.h"
#include "SensorData.h"
#include "sensors/dht22.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;


/** 
 * @brief DHT Task function
 * 
 * @details This function handles DHT sensor operations in a FreeRTOS task. It reads temperature and humidity
 * data from the DHT sensor and sends it to a queue for processing. The task runs in an infinite loop,
 * waiting for data to be available in the queue. When data is received, it is sent to the network for processing.
 * 
 */
void dhtTask(void *parameter)
{
    SensorDHT dhtSensor(DHT_PIN);
    sensor_data_t accData = {};

    dhtSensor.begin();

    while (true)
    {
        dhtSensor.update();
        accData.temperature = dhtSensor.getTemperature();
        accData.humidity = dhtSensor.getHumdity();

        Serial.print("[DHT Task] Temperature: ");
        Serial.println(accData.temperature);
        Serial.print("[DHT Task] Humidity: ");
        Serial.println(accData.humidity);

        if(xQueueSend(dataQueue, &accData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[DHT Task] Failed to send data to queue");
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}