/**
 * @file network.h
 * @brief Network Management Class
 *
 * @details This file contains the declaration of the Network class, which manages
 * WiFi and LTE connections with automatic fallback capabilities.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "utilities.h"
#include <cstdint>
#include <TinyGsmClient.h>

extern Network network;

/**
 * @brief Network management class for WiFi and LTE connectivity
 *
 * @details Provides automatic connection management with WiFi priority and LTE fallback.
 * Includes power management features to disable LTE when WiFi is available.
 */
class Network
{
public:
    Network();
    void begin();

    bool connectWiFi(const char* ssid, const char* password);
    void disconnectWiFi();
    bool isWiFiConnected() const;

    bool connectLTE(const char* apn);
    void disconnectLTE();
    bool isLTEConnected();

    bool isConnected();
    void maintainConnection(const char* ssid, const char* password, const char* apn);

private:
    bool enableModem();
    bool disableModem();

    bool wifiConnected;
    bool lteConnected;
    bool modemEnabled;
};

#endif
