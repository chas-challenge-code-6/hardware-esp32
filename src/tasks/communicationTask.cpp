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
#include "WiFi.h"
#include "network/network.h"
#include "config.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <CustomJWT.h>
#include <HTTPClient.h>
#include <TinyGsmClient.h>
#include <cstring>

extern QueueHandle_t httpQueue;
extern SemaphoreHandle_t modemMutex;

void sendJsonPlain(const char *url, const char *jsonPayload)
{
    HTTPClient http;
    int httpResponseCode = -1;
    if (strncmp(url, "https://", 8) == 0)
    {
        WiFiClientSecure client;
        client.setInsecure();
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        httpResponseCode = http.POST(String(jsonPayload));
    }
    else if (strncmp(url, "http://", 7) == 0)
    {
        WiFiClient client;
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        httpResponseCode = http.POST(String(jsonPayload));
    }
    else
    {
        safePrintln("[CommTask] Invalid URL scheme. Must start with http:// or https://");
        return;
    }

    safePrint("[CommTask] HTTP: ");
    safePrintln(httpResponseCode);
    if (httpResponseCode > 0)
    {
        String response = http.getString();
        if (response.length() > 50)
        {
            safePrint("[CommTask] Response: ");
            safePrintln(response.substring(0, 50) + "...");
        }
        else
        {
            safePrint("[CommTask] Response: ");
            safePrintln(response);
        }
    }
    else
    {
        safePrintln("[CommTask] Error sending HTTP POST");
    }
    http.end();
}

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
    processed_data_t outgoingData;
    memset(&outgoingData, 0, sizeof(outgoingData));

    Network network;

    network.begin();

    char key[] = JWT_TOKEN;
    CustomJWT jwt(key, 256);

    while (true)
    {
        network.maintainConnection(WIFI_SSID, PASSWORD, NETWORK_APN);

        if (xQueueReceive(httpQueue, &outgoingData, pdMS_TO_TICKS(1000)))
        {
            if (network.isWiFiConnected())
            {
                safePrintln("[CommTask] Sending via WiFi...");
                sendJsonPlain(BACKEND_URL, outgoingData.json);
            }
            else if (network.isLTEConnected())
            {
                safePrintln("[CommTask] Sending via LTE...");
                // TODO: Implement LTE sending logic
            }
            else
            {
                safePrintln("[CommTask] No network available, cannot send data.");
            }

            memset(&outgoingData, 0, sizeof(outgoingData));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
