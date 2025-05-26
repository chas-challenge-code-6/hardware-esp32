#include "tasks/networkStatusTask.h"
#include "config.h"
#include "network/network.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0
#define SYSTEM_READY_BIT BIT1

Network network;

void networkStatusTask(void *pvParameters)
{
    bool systemReady = false;
    bool startupLogged = false;

    while (!systemReady)
    {
        if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            EventBits_t bits = xEventGroupWaitBits(networkEventGroup, SYSTEM_READY_BIT, pdFALSE,
                                                   pdTRUE, pdMS_TO_TICKS(100));
            xSemaphoreGive(networkEventMutex);

            if (bits & SYSTEM_READY_BIT)
            {
                systemReady = true;
                safePrintln("[Net Task] System ready, starting network management");
            }
            else
            {
                if (!startupLogged)
                {
                    safePrintln("[Net Task] Waiting for system ready...");
                    startupLogged = true;
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
        else
        {
            safePrintln(
                "[Net Task] Failed to access network event group during startup, retrying...");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    bool lastConnectionState = false;

    while (true)
    {
        network.maintainConnection(WIFI_SSID, PASSWORD, NETWORK_APN);

        if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            bool isConnected = network.isConnected();
            if (isConnected)
            {
                xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
                if (!lastConnectionState)
                {
                    safePrintln("[Net Task] Connected to internet");
                    lastConnectionState = true;
                }
            }
            else
            {
                xEventGroupClearBits(networkEventGroup, NETWORK_CONNECTED_BIT);
                if (lastConnectionState)
                {
                    safePrintln("[Net Task] Connection lost, retrying...");
                    lastConnectionState = false;
                }
            }
            xSemaphoreGive(networkEventMutex);
        }
        else
        {
            safePrintln("[Net Task] Failed to access network event group, continuing...");
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
