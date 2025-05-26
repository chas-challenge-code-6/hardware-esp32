/**
 * @file gasTask.h
 * @brief Gas Task Header File
 *
 * @details This file contains the declaration of the gasTask function, which is used to handle
 * gas sensor operations in a FreeRTOS task. The task is responsible for reading gas concentration
 * data from the MQ2 sensor and sending it to a queue for processing.
 *
 */

#ifndef MQ2TASK_H
#define MQ2TASK_H

/**
 * @brief Gas Task function
 *
 * @param parameter
 */
void gasTask(void *parameter);

#endif