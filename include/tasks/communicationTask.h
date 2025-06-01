/**
 * @file communicationTask.h
 * @brief Communication Task Header File
 *
 * @details This file contains the declaration of the communicationTask function, which is used to
 * handle communication operations in a FreeRTOS task. The task is responsible for sending and
 * receiving data via WiFi and LTE networks with authentication support.
 */

#ifndef COMMUNICATION_TASK_H
#define COMMUNICATION_TASK_H

#include <cstring>
#include <Arduino.h>

/**
 * @brief HTTP response structure
 * 
 * @details Contains the response data from HTTP requests including status code,
 * response body, and success flag.
 */
struct HttpResponse
{
  int code;
  String body;
  bool success;

  /**
   * @brief Default constructor
   */
  HttpResponse() : code(-1), success(false) {}
  
  /**
   * @brief Constructor with response data
   * @param c HTTP status code
   * @param b Response body
   */
  HttpResponse(int c, const String& b) : code(c), body(b), success(c > 0) {}
};

/**
 * @brief Network connection type enumeration
 */
enum class NetworkType
{
  WIFI,
  LTE
};

/**
 * @brief Authentication type enumeration
 */
enum class AuthType
{
  NONE,
  BACKEND_JWT
};

/**
 * @brief Parse authentication response and extract JWT token
 * @param response JSON response string from authentication endpoint
 * @param token Output string to store the extracted JWT token
 * @return true if token was successfully extracted, false otherwise
 */
bool parseAuthResponse(const String& response, String& token);

/**
 * @brief Handle and log HTTP response
 * @param response HTTP response structure containing status and body
 * @param context Description of the request context for logging
 * @param authType Type of authentication used for the request
 */
void handleHttpResponse(const HttpResponse& response, const char* context, AuthType authType = AuthType::NONE);

/**
 * @brief Perform HTTP request via WiFi
 * @param url Target URL for the request
 * @param payload JSON payload to send
 * @param authHeader Authorization header (Bearer token, etc.)
 * @return HttpResponse structure with status code and response body
 */
HttpResponse performWiFiRequest(const char* url, const char* payload, const char* authHeader = "");

/**
 * @brief Perform HTTP request via LTE
 * @param url Target URL for the request
 * @param payload JSON payload to send
 * @param authHeader Authorization header (Bearer token, etc.)
 * @return HttpResponse structure with status code and response body
 */
HttpResponse performLTERequest(const char* url, const char* payload, const char* authHeader = "");

/**
 * @brief Create Bearer authorization header from JWT token
 * @param token JWT token string
 * @return Formatted Bearer authorization header string
 */
String createBearerHeader(const String& token);

/**
 * @brief Send sensor data with automatic authentication
 * @param jsonPayload JSON string containing sensor data
 * 
 * @details Automatically handles backend JWT authentication and token refresh.
 * Falls back to plain HTTP if authentication is disabled.
 */
void sendDataWithAuth(const char* jsonPayload);

/**
 * @brief Send JSON data via HTTP POST without authentication
 * @param url Target URL for the request
 * @param jsonPayload JSON string containing data to send
 * 
 * @details Sends data as plain HTTP POST request without any authentication headers.
 * Used when authentication is disabled or as fallback method.
 */
void sendJsonPlain(const char* url, const char* jsonPayload);

/**
 * @brief Authenticate with backend server and retrieve JWT token
 * @param token Output string to store the retrieved JWT token
 * @return true if authentication successful, false otherwise
 * 
 * @details Sends login credentials to backend authentication endpoint and parses
 * the response to extract JWT token and expiry information. Supports multiple
 * response formats and retry logic on failure.
 */
bool authenticateWithBackend(String& token);

/**
 * @brief Send JSON data with backend-issued JWT authentication
 * @param url Target URL for the request
 * @param jsonPayload JSON string containing data to send
 * @param token JWT token obtained from backend authentication
 * 
 * @details Sends authenticated HTTP POST request using Bearer token in Authorization
 * header. Automatically selects WiFi or LTE based on availability and handles
 * authentication errors including token expiry.
 */
void sendJsonWithBackendJWT(const char* url, const char* jsonPayload, const String& token);

/**
 * @brief Main communication task function for FreeRTOS
 * @param pvParameters Task parameters (unused)
 * 
 * @details Main task loop that handles network communication operations:
 * - Maintains WiFi/LTE connections with automatic fallback
 * - Receives sensor data from queue
 * - Sends data with appropriate authentication method
 * - Handles network failures and authentication errors
 * - Runs continuously with 2-second intervals
 */
void communicationTask(void* pvParameters);

#endif
