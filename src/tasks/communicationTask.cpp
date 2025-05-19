/**
 * @file communicationTask.cpp
 * @brief Communication Task Implementation File
 * 
 * @details This file contains the implementation of the communicationTask function, which is used to handle
 * communication operations in a FreeRTOS task. The task is responsible for sending and receiving data
 * from a queue and processing it. The task runs in an infinite loop, waiting for data to be
 * available in the queue. When data is received, it is sent to the network for processing.
 * 
 */

#include "SensorData.h"
#include "network/network.h"
#include <Arduino.h>

extern QueueHandle_t httpQueue;


/**
 * @brief communicationTask function
 * 
 * @details This function handles communication operations in a FreeRTOS task. It reads processed data
 * from a queue and sends it to the network. The task runs in an infinite loop, waiting for data to
 * be available in the queue. When data is received, it is sent to the network for processing.
 * 
 * @param pvParameters 
 */
void communicationTask(void *pvParameters)
{
    processed_data_t outgoingData; ///< Data structure to hold outgoing data

    while (true)
    {
        if(xQueueReceive(httpQueue, &outgoingData, portMAX_DELAY)) {
            
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
