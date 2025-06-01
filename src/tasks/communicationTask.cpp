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
#include "tasks/communicationTask.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <TinyGSM.h>
#include <TinyGsmClient.h>
#include <cstring>
#include <ArduinoJson.h>

extern QueueHandle_t httpQueue;
extern SemaphoreHandle_t modemMutex;
extern TinyGsm modem;

String currentJWTToken = "";
unsigned long tokenExpiryTime = 0;

String createBearerHeader(const String& token)
{
    return "Bearer " + token;
}

bool parseAuthResponse(const String& response, String& token)
{
    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (error)
    {
        safePrintln("[CommTask] Failed to parse authentication response");
#if DEBUG
        safePrintf("[CommTask] JSON Error: %s\n", error.c_str());
        safePrintf("[CommTask] Response: %s\n", response.c_str());
#endif
        return false;
    }

    if (responseDoc["data"]["token"].is<String>())
    {
        token = responseDoc["data"]["token"].as<String>();

        unsigned long expiresIn = 0;
        if (responseDoc["data"]["expires_in"].is<unsigned long>())
        {
            expiresIn = responseDoc["data"]["expires_in"].as<unsigned long>();
        }
        else if (responseDoc["expires_in"].is<unsigned long>())
        {
            expiresIn = responseDoc["expires_in"].as<unsigned long>();
        }

        tokenExpiryTime = expiresIn > 0 ? millis() + (expiresIn * 1000) : millis() + DEFAULT_TOKEN_EXPIRY_MS;

        safePrintln("[CommTask] Authentication successful");
        return true;
    }
    else if (responseDoc["token"].is<String>())
    {
        token = responseDoc["token"].as<String>();

        unsigned long expiresIn = 0;
        if (responseDoc["expires_in"].is<unsigned long>())
        {
            expiresIn = responseDoc["expires_in"].as<unsigned long>();
        }

        tokenExpiryTime = expiresIn > 0 ? millis() + (expiresIn * 1000) : millis() + DEFAULT_TOKEN_EXPIRY_MS;

        safePrintln("[CommTask] Authentication successful");
        return true;
    }
    else
    {
        safePrintln("[CommTask] No token in authentication response");
        return false;
    }
}

void handleHttpResponse(const HttpResponse& response, const char* context)
{
    safePrintf("[CommTask] HTTP %s: %d\n", context, response.code);

    if (response.success)
    {
        if (response.code == 401)
        {
            safePrintf("[CommTask] AUTHENTICATION ERROR 401 (%s)! Token may be expired or invalid.\n", context);
#if DEBUG
            safePrintf("[CommTask] Full response: %s\n", response.body.c_str());
#endif
            currentJWTToken = "";
            tokenExpiryTime = 0;
        }
        else if (response.body.length() > 50)
        {
#if DEBUG
            safePrintf("[CommTask] Response: %.50s...\n", response.body.c_str());
#endif
        }
        else
        {
#if DEBUG
            safePrintf("[CommTask] Response: %s\n", response.body.c_str());
#endif
        }
    }
    else
    {
        safePrintf("[CommTask] Error sending HTTP POST (%s)\n", context);
    }
}

HttpResponse performWiFiRequest(const char* url, const char* payload, const char* authHeader)
{
    HTTPClient http;
    WiFiClientSecure* secureClient = nullptr;
    WiFiClient* insecureClient = nullptr;
    bool httpStarted = false;
    HttpResponse response;

#if DEBUG
    safePrintf("[CommTask] WiFi request to: %s\n", url);
    safePrintf("[CommTask] Payload size: %d\n", strlen(payload));
#endif

    if (strncmp(url, "https://", 8) == 0)
    {
        secureClient = new WiFiClientSecure();
        secureClient->setInsecure();
        secureClient->setTimeout(AUTH_TIMEOUT_MS / 1000);
        secureClient->setHandshakeTimeout(30);
        httpStarted = http.begin(*secureClient, url);
#if DEBUG
        safePrintln("[CommTask] Using HTTPS client");
#endif
    }
    else if (strncmp(url, "http://", 7) == 0)
    {
        insecureClient = new WiFiClient();
        insecureClient->setTimeout(AUTH_TIMEOUT_MS / 1000);
        httpStarted = http.begin(*insecureClient, url);
#if DEBUG
        safePrintln("[CommTask] Using HTTP client");
#endif
    }
    else
    {
        safePrintln("[CommTask] Invalid URL scheme!");
        return response;
    }

    if (httpStarted)
    {
        http.addHeader("Content-Type", "application/json");
        http.addHeader("User-Agent", "ESP32-Sentinel/1.0");
        if (authHeader && strlen(authHeader) > 0)
        {
            http.addHeader("Authorization", authHeader);
        }
        http.setTimeout(AUTH_TIMEOUT_MS);

        int httpResponseCode = http.POST(payload);
        String responseBody = http.getString();
        response = HttpResponse(httpResponseCode, responseBody);

        http.end();
    }

    if (secureClient)
    {
        delete secureClient;
        secureClient = nullptr;
    }
    if (insecureClient)
    {
        delete insecureClient;
        insecureClient = nullptr;
    }

    return response;
}

HttpResponse performLTERequest(const char* url, const char* payload, const char* authHeader)
{
    HttpResponse response;

    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE)
    {
        safePrintln("[CommTask] Failed to acquire modem mutex for LTE communication");
        return response;
    }

#if DEBUG
    safePrintf("[CommTask] LTE request to: %s\n", url);
    safePrintf("[CommTask] Payload size: %d\n", strlen(payload));
#endif

    if (!modem.https_begin())
    {
        safePrintln("[CommTask] Failed to initialize HTTPS for LTE");
        xSemaphoreGive(modemMutex);
        return response;
    }

    if (!modem.https_set_url(url))
    {
        safePrintln("[CommTask] Failed to set URL for LTE request");
        modem.https_end();
        xSemaphoreGive(modemMutex);
        return response;
    }

    modem.https_set_accept_type("application/json");
    modem.https_add_header("Content-Type", "application/json");
    modem.https_set_user_agent("ESP32-Sentinel/1.0");
    if (authHeader && strlen(authHeader) > 0)
    {
        modem.https_add_header("Authorization", authHeader);
    }

    int httpCode = modem.https_post(payload);
    String responseBody = modem.https_body();
    response = HttpResponse(httpCode, responseBody);

    modem.https_end();
    xSemaphoreGive(modemMutex);

    return response;
}

void sendDataWithAuth(const char* jsonPayload)
{
    String dataUrl = String(BACKEND_URL) + API_ENDPOINT;

    if (currentJWTToken.isEmpty() || millis() > tokenExpiryTime - TOKEN_REFRESH_MARGIN_MS)
    {
        safePrintln("[CommTask] Refreshing backend JWT token...");
        authenticateWithBackend(currentJWTToken);
    }
    sendJson(dataUrl.c_str(), jsonPayload, currentJWTToken);
}

bool authenticateWithBackend(String& token)
{
    Network network;
    bool success = false;
    int attempts = 0;

    String authUrl = String(BACKEND_URL) + AUTH_ENDPOINT;

    JsonDocument loginDoc;
    loginDoc["username"] = AUTH_USERNAME;
    loginDoc["password"] = AUTH_PASSWORD;

    String loginPayload;
    serializeJson(loginDoc, loginPayload);

#if DEBUG
    safePrintf("[CommTask] Authenticating with backend at: %s\n", authUrl.c_str());
    safePrintf("[CommTask] Auth payload: %s\n", loginPayload.c_str());
#endif

    while (attempts < AUTH_RETRY_ATTEMPTS && !success)
    {
        attempts++;
        if (attempts > 1)
        {
#if DEBUG
            safePrintf("[CommTask] Authentication attempt %d of %d\n", attempts, AUTH_RETRY_ATTEMPTS);
#endif
        }

        HttpResponse response;

        if (network.isWiFiConnected())
        {
            response = performWiFiRequest(authUrl.c_str(), loginPayload.c_str());
        }
        else if (network.isLTEConnected())
        {
            response = performLTERequest(authUrl.c_str(), loginPayload.c_str());
        }
        else
        {
            safePrintln("[CommTask] No network available for authentication");
            break;
        }

        if (response.code == 200)
        {
            success = parseAuthResponse(response.body, token);
        }
        else if (response.code == 401)
        {
            safePrintf("[CommTask] Authentication failed with code: %d\n", response.code);
#if DEBUG
            safePrintf("[CommTask] Error response: %s\n", response.body.c_str());
#endif
            break;
        }
        else
        {
            safePrintf("[CommTask] Authentication failed with code: %d\n", response.code);
#if DEBUG
            safePrintf("[CommTask] Error response: %s\n", response.body.c_str());
#endif
        }

        if (attempts < AUTH_RETRY_ATTEMPTS && !success)
        {
#if DEBUG
            safePrintln("[CommTask] Retrying authentication in 2 seconds...");
#endif
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

    if (!success && attempts >= AUTH_RETRY_ATTEMPTS)
    {
        safePrint("[CommTask] Authentication failed after ");
        safePrint(AUTH_RETRY_ATTEMPTS);
        safePrintln(" attempts");
    }

    return success;
}

void sendJson(const char* url, const char* jsonPayload, const String& token)
{
    Network network;
    String authHeader = createBearerHeader(token);
    HttpResponse response;

#if DEBUG
    safePrint("[CommTask] Using backend JWT token: ");
    safePrintln(token.substring(0, 20) + "...");
#endif

    if (network.isWiFiConnected())
    {
        response = performWiFiRequest(url, jsonPayload, authHeader.c_str());
        handleHttpResponse(response, "WiFi (Backend)");
    }
    else if (network.isLTEConnected())
    {
        response = performLTERequest(url, jsonPayload, authHeader.c_str());
        handleHttpResponse(response, "LTE (Backend)");
    }
    else
    {
        safePrintln("[CommTask] No network available for backend communication");
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
void communicationTask(void* pvParameters)
{
    processed_data_t outgoingData;
    memset(&outgoingData, 0, sizeof(outgoingData));

    Network network;
    network.begin();

    while (true)
    {
        network.maintainConnection(WIFI_SSID, PASSWORD, NETWORK_APN);

        if (xQueueReceive(httpQueue, &outgoingData, pdMS_TO_TICKS(1000)))
        {
            if (network.isWiFiConnected())
            {
#if DEBUG
                safePrintln("[CommTask] Sending via WiFi...");
#endif
                sendDataWithAuth(outgoingData.json);
            }
            else if (network.isLTEConnected())
            {
#if DEBUG
                safePrintln("[CommTask] Sending via LTE...");
#endif
                sendDataWithAuth(outgoingData.json);
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
