#include "network/network.h"
#include "main.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <TinyGSM.h>
#include <WiFi.h>

extern TinyGsm modem;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

Network::Network() : wifiConnected(false), lteConnected(false) {}

void Network::begin()
{
    // vTaskDelay(pdMS_TO_TICKS(10));

    // enableModem();
    // vTaskDelay(pdMS_TO_TICKS(10));
    safePrintln("[Network] Network hardware initialized (WiFi + LTE modem)");
}

bool Network::connectWiFi(const char *ssid, const char *password)
{
#if DEBUG
    safePrintln("NETWORK: Connecting to WiFi...");
#endif
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    wifiConnected = (WiFi.status() == WL_CONNECTED);
#if DEBUG
    safePrintln("NETWORK: Connected to WiFI.");
#endif
    return wifiConnected;
}

void Network::disconnectWiFi()
{
    WiFi.disconnect();
    wifiConnected = false;
}

bool Network::isWiFiConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

bool Network::enableModem() const
{

// Enable DC boost
#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif

// Set modem reset
#ifdef MODEM_RESET_PIN
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);
#endif

    // Turn on the modem
    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    delay(3000);

    // Thread-safe modem initialization
    bool initResult = false;
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE)
    {
        initResult = modem.init();
        if (!initResult)
        {
            DBG("Failed to restart modem");
            xSemaphoreGive(modemMutex);
            return false;
        }

#ifndef TINY_GSM_MODEM_SIM7672
        bool ret = modem.setNetworkMode(MODEM_NETWORK_AUTO);
        if (modem.waitResponse(10000L) != 1)
        {
            DBG(" setNetworkMode fail");
            xSemaphoreGive(modemMutex);
            return false;
        }
#endif
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[Network] Failed to acquire modem mutex for initialization");
        return false;
    }

    return initResult;
}

bool Network::connectLTE(const char *apn)
{
    bool result = false;
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE)
    {
        modem.restart();
        lteConnected = modem.gprsConnect(apn);
        result = lteConnected;
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[Network] Failed to acquire modem mutex for LTE connection");
    }
    return result;
}

void Network::disconnectLTE()
{
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE)
    {
        modem.gprsDisconnect();
        lteConnected = false;
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[Network] Failed to acquire modem mutex for LTE disconnection");
    }
}

bool Network::isLTEConnected()
{
    bool result = false;
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        result = modem.isNetworkConnected();
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[Network] Failed to acquire modem mutex to check LTE status");
    }
    return result;
}

bool Network::isConnected()
{
    return isWiFiConnected() || isLTEConnected();
}

void Network::maintainConnection(const char *ssid, const char *password, const char *apn)
{
    if (isWiFiConnected())
    {
        // Already connected to WiFi, nothing to do
    }
    else
    {
        int n = WiFi.scanNetworks(false, false, false, 300, 0, ssid);
        bool ssidFound = (n > 0);
        
        if (ssidFound)
        {
            safePrintln("[Network] Target WiFi network found, attempting connection...");
            connectWiFi(ssid, password);
        }
        else
        {
            safePrintln("[Network] WiFi network not available, checking LTE...");
            if (!isLTEConnected())
            {
                safePrintln("[Network] Attempting LTE connection...");
                connectLTE(apn);
            }
        }
    }

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (isConnected())
        {
            xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
        }
        else
        {
            xEventGroupClearBits(networkEventGroup, NETWORK_CONNECTED_BIT);
        }
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("[Network] Failed to access network event group in maintainConnection");
    }
}
