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

struct HttpResponse
{
    int code;
    String body;
    bool success;

    HttpResponse() : code(-1), success(false) {}
    HttpResponse(int c, const String& b) : code(c), body(b), success(c > 0) {}
};

enum class NetworkType
{
    WIFI,
    LTE
};

enum class AuthType
{
    NONE,
    BACKEND_JWT
};

bool parseAuthResponse(const String& response, String& token);
void handleHttpResponse(const HttpResponse& response, const String& context, AuthType authType = AuthType::NONE);
HttpResponse performWiFiRequest(const char* url, const String& payload, const String& authHeader = "");
HttpResponse performLTERequest(const char* url, const String& payload, const String& authHeader = "");
String createBearerHeader(const String& token);
bool authenticateWithBackend(String& token);
void sendJsonWithBackendJWT(const char* url, const char* jsonPayload, const String& token);
void sendJsonPlain(const char* url, const char* jsonPayload);

void sendDataWithAuth(const char* jsonPayload);

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
        safePrint("[CommTask] JSON Error: ");
        safePrintln(error.c_str());
        safePrint("[CommTask] Response: ");
        safePrintln(response);
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

void handleHttpResponse(const HttpResponse& response, const String& context, AuthType authType)
{
    safePrint("[CommTask] HTTP ");
    safePrint(context);
    safePrint(": ");
    safePrintln(response.code);

    if (response.success)
    {
        if (response.code == 401)
        {
            safePrint("[CommTask] AUTHENTICATION ERROR 401 (");
            safePrint(context);
            safePrintln(")! Token may be expired or invalid.");
#if DEBUG
            safePrint("[CommTask] Full response: ");
            safePrintln(response.body);
#endif

            if (authType == AuthType::BACKEND_JWT)
            {
                currentJWTToken = "";
                tokenExpiryTime = 0;
            }
        }
        else if (response.body.length() > 50)
        {
#if DEBUG
            safePrint("[CommTask] Response: ");
            safePrintln(response.body.substring(0, 50) + "...");
#endif
        }
        else
        {
#if DEBUG
            safePrint("[CommTask] Response: ");
            safePrintln(response.body);
#endif
        }
    }
    else
    {
        safePrint("[CommTask] Error sending HTTP POST (");
        safePrint(context);
        safePrintln(")");
    }
}

HttpResponse performWiFiRequest(const char* url, const String& payload, const String& authHeader)
{
    HTTPClient http;
    WiFiClientSecure* secureClient = nullptr;
    WiFiClient* insecureClient = nullptr;
    bool httpStarted = false;
    HttpResponse response;

#if DEBUG
    safePrint("[CommTask] WiFi request to: ");
    safePrintln(url);
    safePrint("[CommTask] Payload size: ");
    safePrintln(payload.length());
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
        if (!authHeader.isEmpty())
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

HttpResponse performLTERequest(const char* url, const String& payload, const String& authHeader)
{
    HttpResponse response;

    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE)
    {
        safePrintln("[CommTask] Failed to acquire modem mutex for LTE communication");
        return response;
    }

#if DEBUG
    safePrint("[CommTask] LTE request to: ");
    safePrintln(url);
    safePrint("[CommTask] Payload size: ");
    safePrintln(payload.length());
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
    if (!authHeader.isEmpty())
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

#ifdef USE_BACKEND_AUTH
    if (currentJWTToken.isEmpty() || millis() > tokenExpiryTime - TOKEN_REFRESH_MARGIN_MS)
    {
        safePrintln("[CommTask] Refreshing backend JWT token...");
        authenticateWithBackend(currentJWTToken);
    }
    sendJsonWithBackendJWT(dataUrl.c_str(), jsonPayload, currentJWTToken);
#else
    sendJsonPlain(dataUrl.c_str(), jsonPayload);
#endif
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
    safePrint("[CommTask] Authenticating with backend at: ");
    safePrintln(authUrl);
    safePrint("[CommTask] Auth payload: ");
    safePrintln(loginPayload);
#endif

    while (attempts < AUTH_RETRY_ATTEMPTS && !success)
    {
        attempts++;
        if (attempts > 1)
        {
#if DEBUG
            safePrint("[CommTask] Authentication attempt ");
            safePrint(attempts);
            safePrint(" of ");
            safePrintln(AUTH_RETRY_ATTEMPTS);
#endif
        }

        HttpResponse response;

        if (network.isWiFiConnected())
        {
            response = performWiFiRequest(authUrl.c_str(), loginPayload);
        }
        else if (network.isLTEConnected())
        {
            response = performLTERequest(authUrl.c_str(), loginPayload);
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
            safePrint("[CommTask] Authentication failed with code: ");
            safePrintln(response.code);
#if DEBUG
            safePrint("[CommTask] Error response: ");
            safePrintln(response.body);
#endif
            break;
        }
        else
        {
            safePrint("[CommTask] Authentication failed with code: ");
            safePrintln(response.code);
#if DEBUG
            safePrint("[CommTask] Error response: ");
            safePrintln(response.body);
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

void sendJsonWithBackendJWT(const char* url, const char* jsonPayload, const String& token)
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
        response = performWiFiRequest(url, String(jsonPayload), authHeader);
        handleHttpResponse(response, "WiFi (Backend JWT)", AuthType::BACKEND_JWT);
    }
    else if (network.isLTEConnected())
    {
        response = performLTERequest(url, String(jsonPayload), authHeader);
        handleHttpResponse(response, "LTE (Backend JWT)", AuthType::BACKEND_JWT);
    }
    else
    {
        safePrintln("[CommTask] No network available for backend JWT communication");
    }
}

void sendJsonPlain(const char* url, const char* jsonPayload)
{
    Network network;
    HttpResponse response;

    if (network.isWiFiConnected())
    {
        response = performWiFiRequest(url, String(jsonPayload));
        handleHttpResponse(response, "WiFi (Plain)");
    }
    else if (network.isLTEConnected())
    {
        response = performLTERequest(url, String(jsonPayload));
        handleHttpResponse(response, "LTE (Plain)");
    }
    else
    {
        safePrintln("[CommTask] No network available for communication");
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
