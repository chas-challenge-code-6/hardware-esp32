#include "network/wifi.h"
#include <Arduino.h>

void WiFiTask(void* pvParameters) {
    WiFiClient* wifiClient = static_cast<WiFiClient*>(pvParameters);
    while (true) {
        if (!wifiClient->isConnected()) {
            wifiClient->begin();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
