/**
 * @file dhtTask.h
 * @brief DHT Task Header File
 *
 * @details This file contains the declaration of the dhtTask function, which is used to handle
 * DHT sensor operations in a FreeRTOS task. The task is responsible for reading temperature
 * and humidity data from the DHT sensor and sending it to a queue for processing.
  The task runs in an infinite loop, waiting for data to be available in the queue.
 */

#ifndef DHTTASK
#define DHTTASK

/**
 * @brief DHT Task function
 *
 * @details This function handles DHT sensor operations in a FreeRTOS task. It reads temperature and
 * humidity data from the DHT sensor and sends it to a queue for processing. The task runs in an
 * infinite loop, waiting for data to be available in the queue. When data is received, it is sent
 * to the network for processing.
 *
 */
void dhtTask(void *parameter);

#endif