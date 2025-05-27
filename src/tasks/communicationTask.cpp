/**
 * @file communicationTask.cpp
 * @brief Communication Task Implementation File
 *
 * @details This file contains the implementation of the communicationTask function, which is used
 * to handle communication operations in a FreeRTOS task. The task is responsible for sending and
 * receiving data from a queue and processing it. The task runs in an infinite loop, waiting for
 * data to be available in the queue. When data is received, it is sent to the network for
 * processing.
 *
 */

#include "SensorData.h"
#include "WiFi.h"
#include "config.h"
#include "network/network.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <CustomJWT.h>
#include <HTTPClient.h>
#include <TinyGSM.h>
#include <TinyGsmClient.h>
#include <cstring>

extern QueueHandle_t httpQueue;
extern SemaphoreHandle_t modemMutex;
extern TinyGsm modem;

// Den här har dubbel kod och diverse tester för LTE, fixa sen!

void sendJsonPlainWiFi(const char *url, const char *jsonPayload)
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

    safePrint("[CommTask] HTTP WiFi: ");
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
        safePrintln("[CommTask] Error sending HTTP POST over WiFi");
    }
    http.end();
}

void sendJsonPlainLTE(const char *url, const char *jsonPayload)
{
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE)
    {
        safePrint("[CommTask] LTE Plain HTTP POST to: ");
        safePrintln(url);
        safePrint("[CommTask] Payload size: ");
        safePrintln(strlen(jsonPayload));

        if (!modem.https_begin())
        {
            safePrintln("[CommTask] Failed to initialize HTTPS for LTE");
            xSemaphoreGive(modemMutex);
            return;
        }

        // Set URL
        if (!modem.https_set_url(url))
        {
            safePrintln("[CommTask] Failed to set URL for LTE request");
            modem.https_end();
            xSemaphoreGive(modemMutex);
            return;
        }

        modem.https_set_accept_type("application/json");
        modem.https_add_header("Content-Type", "application/json");
        modem.https_set_user_agent("ESP32-Sentinel/1.0");

        int httpCode = modem.https_post(String(jsonPayload));

        safePrint("[CommTask] HTTP LTE: ");
        safePrintln(httpCode);

        if (httpCode == 200)
        {
            String response = modem.https_body();
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
            safePrint("[CommTask] HTTP error code: ");
            safePrintln(httpCode);
            String response = modem.https_body();
            if (response.length() > 0)
            {
                safePrint("[CommTask] Error response: ");
                safePrintln(response);
            }
        }

        modem.https_end();
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[CommTask] Failed to acquire modem mutex for LTE communication");
    }
}

void sendJsonPlain(const char *url, const char *jsonPayload)
{
    Network network;

    if (network.isWiFiConnected())
    {
        sendJsonPlainWiFi(url, jsonPayload);
    }
    else if (network.isLTEConnected())
    {
        sendJsonPlainLTE(url, jsonPayload);
    }
    else
    {
        safePrintln("[CommTask] No network available for communication");
    }
}

void sendJsonJWTWiFi(const char *url, const char *jsonPayload, CustomJWT &jwt)
{
    HTTPClient http;
    int httpResponseCode = -1;

    char jwtClaims[256];
    snprintf(jwtClaims, sizeof(jwtClaims),
             "{\"iss\":\"esp32-sensor\",\"sub\":\"sensor-data\",\"device_id\":\"%s\"}", DEVICE_ID);

    if (!jwt.encodeJWT(jwtClaims))
    {
        safePrintln("[CommTask] Failed to encode JWT (WiFi)");
        safePrint("[CommTask] JWT Claims: ");
        safePrintln(jwtClaims);
        return;
    }

    String token = String(jwt.out);
    safePrint("[CommTask] JWT Token (WiFi): ");
    safePrintln(token.substring(0, 20) + "..."); // debug

    if (strncmp(url, "https://", 8) == 0)
    {
        WiFiClientSecure client;
        client.setInsecure();
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + token);
        safePrint("[CommTask] WiFi HTTPS Request to: ");
        safePrintln(url);
        safePrint("[CommTask] Payload size: ");
        safePrintln(strlen(jsonPayload));
        httpResponseCode = http.POST(String(jsonPayload));
    }
    else if (strncmp(url, "http://", 7) == 0)
    {
        WiFiClient client;
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + token);
        safePrint("[CommTask] WiFi HTTP Request to: ");
        safePrintln(url);
        safePrint("[CommTask] Payload size: ");
        safePrintln(strlen(jsonPayload));
        httpResponseCode = http.POST(String(jsonPayload));
    }
    else
    {
        safePrintln("[CommTask] Invalid URL scheme. Must start with http:// or https://");
        return;
    }

    safePrint("[CommTask] HTTP WiFi (JWT): ");
    safePrintln(httpResponseCode);
    if (httpResponseCode > 0)
    {
        String response = http.getString();
        if (httpResponseCode == 401)
        {
            safePrintln("[CommTask] AUTHENTICATION ERROR 401!");
            safePrint("[CommTask] Full response: ");
            safePrintln(response);
        }
        else if (response.length() > 50)
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
        safePrintln("[CommTask] Error sending HTTP POST with JWT over WiFi");
    }
    http.end();
}

void sendJsonJWTLTE(const char *url, const char *jsonPayload, CustomJWT &jwt)
{
    char jwtClaims[256];
    snprintf(jwtClaims, sizeof(jwtClaims),
             "{\"iss\":\"esp32-sensor\",\"sub\":\"sensor-data\",\"device_id\":\"%s\"}", DEVICE_ID);

    if (!jwt.encodeJWT(jwtClaims))
    {
        safePrintln("[CommTask] Failed to encode JWT (LTE)");
        safePrint("[CommTask] JWT Claims: ");
        safePrintln(jwtClaims);
        return;
    }

    String token = String(jwt.out);
    safePrint("[CommTask] JWT Token (LTE): ");
    safePrintln(token.substring(0, 20) + "...");

    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE)
    {
        safePrint("[CommTask] LTE JWT HTTP POST to: ");
        safePrintln(url);
        safePrint("[CommTask] Payload size: ");
        safePrintln(strlen(jsonPayload));

        if (!modem.https_begin())
        {
            safePrintln("[CommTask] Failed to initialize HTTPS for LTE JWT");
            xSemaphoreGive(modemMutex);
            return;
        }

        if (!modem.https_set_url(url))
        {
            safePrintln("[CommTask] Failed to set URL for LTE JWT request");
            modem.https_end();
            xSemaphoreGive(modemMutex);
            return;
        }

        modem.https_set_accept_type("application/json");
        modem.https_add_header("Content-Type", "application/json");
        modem.https_add_header("Authorization", "Bearer " + token);
        modem.https_set_user_agent("ESP32-Sentinel/1.0");

        int httpCode = modem.https_post(String(jsonPayload));

        safePrint("[CommTask] HTTP LTE (JWT): ");
        safePrintln(httpCode);

        if (httpCode == 200)
        {
            String response = modem.https_body();
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
        else if (httpCode == 401)
        {
            safePrintln("[CommTask] AUTHENTICATION ERROR 401 (LTE)!");
            String response = modem.https_body();
            safePrint("[CommTask] Full response: ");
            safePrintln(response);
        }
        else
        {
            safePrint("[CommTask] HTTP error code: ");
            safePrintln(httpCode);
            String response = modem.https_body();
            if (response.length() > 0)
            {
                safePrint("[CommTask] Error response: ");
                safePrintln(response);
            }
        }

        modem.https_end();
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[CommTask] Failed to acquire modem mutex for LTE JWT communication");
    }
}

void sendJsonJWT(const char *url, const char *jsonPayload, CustomJWT &jwt)
{
    Network network;

    if (network.isWiFiConnected())
    {
        sendJsonJWTWiFi(url, jsonPayload, jwt);
    }
    else if (network.isLTEConnected())
    {
        sendJsonJWTLTE(url, jsonPayload, jwt);
    }
    else
    {
        safePrintln("[CommTask] No network available for JWT communication");
    }
}

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
void communicationTask(void *pvParameters)
{
    processed_data_t outgoingData;
    memset(&outgoingData, 0, sizeof(outgoingData));

    Network network;

    network.begin();

    char key[] = JWT_TOKEN;
    CustomJWT jwt(key, 256);

    jwt.allocateJWTMemory();

    while (true)
    {
        network.maintainConnection(WIFI_SSID, PASSWORD, NETWORK_APN);

        if (xQueueReceive(httpQueue, &outgoingData, pdMS_TO_TICKS(1000)))
        {
            if (network.isWiFiConnected())
            {
                safePrintln("[CommTask] Sending via WiFi...");
#ifdef USE_JWT_AUTH
                sendJsonJWT(BACKEND_URL, outgoingData.json, jwt);
#else
                sendJsonPlain(BACKEND_URL, outgoingData.json);
#endif
            }
            else if (network.isLTEConnected())
            {
                safePrintln("[CommTask] Sending via LTE...");
#ifdef USE_JWT_AUTH
                sendJsonJWT(BACKEND_URL, outgoingData.json, jwt);
#else
                sendJsonPlain(BACKEND_URL, outgoingData.json);
#endif
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
