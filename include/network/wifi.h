#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiClient {
public:
    WiFiClient(const char* ssid, const char* password);
    void begin();
    bool isConnected();

private:
    const char* ssid;
    const char* password;
};

#endif