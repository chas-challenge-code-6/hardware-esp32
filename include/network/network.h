/**
 * @file network.h
 * @brief Network Management Header File
 * 
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>

/**
 * @brief Network class for managing WiFi and LTE connections
 * 
 * @details This class provides methods to manage network connections, including enabling/disabling the modem,
 * connecting to WiFi and LTE, checking connection status, and maintaining connections.
 * 
 */

class Network
{
public:
    Network();
    void begin();
    bool enableModem() const;
    bool disableModem() const;
    bool connectWiFi(const char *ssid, const char *password);
    bool connectLTE(const char *apn);
    bool isWiFiConnected() const;
    bool isLTEConnected();
    bool isConnected();
    void disconnectWiFi();
    void disconnectLTE();
    void maintainConnection(const char *ssid, const char *password, const char *apn);

private:
    bool wifiConnected;
    bool lteConnected;
};

#endif
