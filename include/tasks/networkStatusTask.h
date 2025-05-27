/**
 * @file networkStatusTask.h
 * @brief Network Status Task Header File
 * 
 * @details This file contains the declaration of the networkStatusTask function, which is used to handle
 * network status monitoring operations in a FreeRTOS task. The task is responsible for checking the
 * network connection status and updating the network event group accordingly. It also handles modem
 * reconnection logic and manages the network event mutex to ensure thread safety.
 */

#ifndef NETWORKSTATUSTASK_H
#define NETWORKSTATUSTASK_H


/**
 * @brief Network Status Task function
 * 
 * @param pvParameters 
 */
void networkStatusTask(void *pvParameters);

#endif