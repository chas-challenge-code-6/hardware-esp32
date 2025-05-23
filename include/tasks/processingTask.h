/**
 * @file processingTask.h
 * @brief Processing Task Header File
 * @details This file contains the declaration of the processingTask function, which is used to handle
 * processing operations in a FreeRTOS task. The task is responsible for processing sensor data
 * and creating a JSON string for network transmission.
 * 
 */

#ifndef PROCESSINGTASK_H
#define PROCESSINGTASK_H


/**
 * @brief Processing Task function
 * 
 * @param pvParameters 
 */
void processingTask(void *pvParameters);

#endif
