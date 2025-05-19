#include "network/network.h"
#include "secrets.h"
#include <Arduino.h>

extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

Network network;

void networkStatusTask(void *pvParameters)
{
    while (true)
    {
        network.maintainConnection(SSID, PASSWORD, NETWORK_APN);
        if (network.isConnected())
        {
            xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
            Serial.println("[NetworkStatusTask] Connected to internet");
        }
        else
        {
            xEventGroupClearBits(networkEventGroup, NETWORK_CONNECTED_BIT);
            Serial.println("[NetworkStatusTask] Not connected, retrying...");
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
