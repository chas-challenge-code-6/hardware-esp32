/**
 * @file communicationTask.h
 * @brief Communication Task Header File
 * 
 * @details This file contains the declaration of the communicationTask function, which is used to handle
 * communication operations in a FreeRTOS task. The task is responsible for sending and receiving data
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H


/**
 * @brief communicationTask function
 * 
 * @details This function handles communication operations in a FreeRTOS task. It reads processed data
 * from a queue and sends it to the network. The task runs in an infinite loop, waiting for data to
 * be available in the queue. When data is received, it is sent to the network for processing.
 * 
 * @param pvParameters 
 */
void communicationTask(void* pvParameters);

#endif
