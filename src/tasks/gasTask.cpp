/**
 * @file gasTask.cpp
 * @brief Gas Task Implementation File
 * 
 * @details This file contains the implementation of the gasTask function, which is used to handle
 * gas sensor operations in a FreeRTOS task. The task is responsible for reading gas concentration
 * data from the MQ2 sensor and sending it to a queue for processing.
 * 
 */

#include "tasks/gasTask.h"
#include "SensorData.h"
#include "sensors/mq2.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;


/**
 * @brief Gas Task function
 * 
 * @details This function handles gas sensor operations in a FreeRTOS task. It reads gas concentration
 * data from the MQ2 sensor and sends it to a queue for processing. The task runs in an infinite loop,
 * waiting for data to be available in the queue. When data is received, it is sent to the network for processing.
 * The task uses the MQ2 sensor to measure gas concentration in parts per million (PPM) and sends the data to a queue.
 * 
 * @param parameter 
 */
void gasTask(void *parameter)
{
    sensor_data_t gasData = {};
    MQ2Sensor gasSensor(GAS_PIN);

    while (true)
    {
        gasSensor.update();

        gasData.gasLevel = gasSensor.getValue();

        Serial.printf("[Gas Task] Gas Concentration (PPM): %.2f\n", gasData.gasLevel);

        if(xQueueSend(dataQueue, &gasData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[Gas Task] Failed to send data to queue");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}