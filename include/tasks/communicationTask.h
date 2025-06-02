/**
 * @file communicationTask.h
 * @brief Communication Task Header File
 *
 * @details This file contains the declaration of the communicationTask function, which is used to
 * handle communication operations in a FreeRTOS task. The task is responsible for sending and
 * receiving data
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <cstring>
#include <CustomJWT.h>
#include <Arduino.h>

 /**
  * @brief Send JSON data via HTTP POST
  * @param url The target URL
  * @param jsonPayload The JSON data to send
  */
void sendJsonPlain(const char* url, const char* jsonPayload);

/**
 * @brief Send JSON data via HTTP POST with JWT authentication
 * @param url The target URL
 * @param jsonPayload The JSON data to send
 * @param jwt JWT instance for authentication
 */
void sendJsonJWT(const char* url, const char* jsonPayload, CustomJWT& jwt);

/**
 * @brief Authenticate with backend and retrieve JWT token
 * @param token Output string to store the retrieved JWT token
 * @param tokenSize Maximum size of the token buffer
 * @return true if authentication successful, false otherwise
 */
bool authenticateWithBackend(String& token);

/**
 * @brief Send JSON data via HTTP POST with backend-issued JWT token
 * @param url The target URL
 * @param jsonPayload The JSON data to send
 * @param token The JWT token from backend authentication
 */
void sendJsonWithBackendJWT(const char* url, const char* jsonPayload, const String& token);

/**
 * @brief communicationTask function
 *
 * @details This function handles communication operations in a FreeRTOS task. It reads processed
 * data from a queue and sends it to the network. The task runs in an infinite loop, waiting for
 * data to be available in the queue. When data is received, it is sent to the network for
 * processing.
 *
 * @param pvParameters
 */
void communicationTask(void* pvParameters);

#endif
