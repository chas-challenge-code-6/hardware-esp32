#ifndef NETWORK_H
#define NETWORK_H

#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>

class Network
{
public:
    Network();
    void begin();
    bool enableModem();
    bool disableModem();
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
    bool modemEnabled;
};

#endif
