#include "network/network.h"
#include "main.h"
#include <Arduino.h>
#include <TinyGSM.h>
#include <WiFi.h>

extern TinyGsm modem;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

Network::Network() : wifiConnected(false), lteConnected(false) {}

void Network::begin()
{
    // vTaskDelay(pdMS_TO_TICKS(10));

    // enableModem();
    // vTaskDelay(pdMS_TO_TICKS(10));
    Serial.println("[Network] Network hardware initialized (WiFi + LTE modem)");
}

bool Network::connectWiFi(const char *ssid, const char *password)
{
#if DEBUG
    Serial.println("NETWORK: Connecting to WiFi...");
#endif
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    wifiConnected = (WiFi.status() == WL_CONNECTED);
#if DEBUG
    Serial.println("NETWORK: Connected to WiFI.");
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

    if (!modem.init())
    {
        DBG("Failed to restart modem");
        return false;
    }

#ifndef TINY_GSM_MODEM_SIM7672
    bool ret;
    ret = modem.setNetworkMode(MODEM_NETWORK_AUTO);
    if (modem.waitResponse(10000L) != 1)
    {
        DBG(" setNetworkMode fail");
        return false;
    }
#endif

    return true;
}

bool Network::connectLTE(const char *apn)
{
    modem.restart();
    lteConnected = modem.gprsConnect(apn);
    return lteConnected;
}

void Network::disconnectLTE()
{
    modem.gprsDisconnect();
    lteConnected = false;
}

bool Network::isLTEConnected()
{
    return modem.isNetworkConnected();
}

bool Network::isConnected() /* not const */
{
    return isWiFiConnected() || isLTEConnected();
}

void Network::maintainConnection(const char *ssid, const char *password, const char *apn)
{
    if (isWiFiConnected())
    {
        // nothing
    }
    else
    {
        int n = WiFi.scanNetworks();
        bool ssidFound = false;
        for (int i = 0; i < n; ++i)
        {
            if (WiFi.SSID(i) == ssid)
            {
                ssidFound = true;
                break;
            }
        }
        if (ssidFound)
        {
            connectWiFi(ssid, password);
        }
        else
        {
            if (!isLTEConnected())
            {
                connectLTE(apn);
            }
        }
    }

    if (isConnected())
    {
        xEventGroupSetBits(networkEventGroup, NETWORK_CONNECTED_BIT);
    }
    else
    {
        xEventGroupClearBits(networkEventGroup, NETWORK_CONNECTED_BIT);
    }
}
