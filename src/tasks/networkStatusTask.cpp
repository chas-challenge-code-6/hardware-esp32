#include "tasks/networkStatusTask.h"
#include "network/network.h"
#include "secrets.h"
#include <Arduino.h>

extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0
#define SYSTEM_READY_BIT BIT1

Network network;

void networkStatusTask(void *pvParameters)
{
    xEventGroupWaitBits(networkEventGroup, SYSTEM_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

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
