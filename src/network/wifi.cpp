#include "network/wifi.h"
#include <WiFi.h>

WiFiClient::WiFiClient(const char* ssid, const char* password) : ssid(ssid), password(password) {}

void WiFiClient::begin() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

bool WiFiClient::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}
