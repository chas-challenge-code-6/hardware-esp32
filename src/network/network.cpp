#include "network/network.h"
#include "config.h"
#include "utilities.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <TinyGSM.h>
#include <WiFi.h>
#include <cstdint>
#include <sys/types.h>

extern TinyGsm modem;
extern EventGroupHandle_t networkEventGroup;
// extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

Network::Network() : wifiConnected(false), lteConnected(false), modemEnabled(false) {}

void Network::begin()
{
    safePrintln("[Network] Network hardware initialized (WiFi ready, LTE modem on-demand)");
}

bool Network::connectWiFi(const char* ssid, const char* password)
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

bool Network::enableModem()
{
    if (modemEnabled)
    {
#if DEBUG
        safePrintln("[Network] Modem already enabled");
#endif
        return true;
    }

    safePrintln("[Network] Initializing modem...");

#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#if DEBUG
    safePrintln("[Network] Modem power enabled");
#endif
#endif

#ifdef MODEM_RESET_PIN
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(10));
    digitalWrite(MODEM_RESET_PIN, HIGH);
#if DEBUG
    safePrintln("[Network] Modem reset sequence completed");
#endif
#endif

    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
#if DEBUG
    safePrintln("[Network] Modem power key sequence completed");
#endif

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
#if DEBUG
    safePrintln("[Network] Modem serial initialized at 115200 baud");
#endif

    vTaskDelay(pdMS_TO_TICKS(3000));

#if DEBUG
    safePrintln("[Network] Testing modem responsiveness...");
#endif
    int attempts = 0;
    while (attempts < 10 && !modem.testAT(1000))
    {
#if DEBUG
        safePrintf("[Network] Modem not responding, attempt %d\n", attempts + 1);
#endif
        attempts++;
        if (attempts > 3)
        {
            // Try power cycle if initial attempts fail
            digitalWrite(BOARD_PWRKEY_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(BOARD_PWRKEY_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(1000));
            digitalWrite(BOARD_PWRKEY_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    if (attempts >= 10)
    {
        safePrintln("[Network] Modem failed to respond to AT commands after 10 attempts");
        return false;
    }
#if DEBUG
    safePrintln("[Network] Modem is responding to AT commands");
#endif

    if (!modem.init())
    {
        safePrintln("[Network] Failed to initialize modem");
        return false;
    }

#if DEBUG
    String name = modem.getModemName();
    String info = modem.getModemInfo();
    safePrintf("[Network] Modem Name: %s\n", name.c_str());
    safePrintf("[Network] Modem Info: %s\n", info.c_str());
#endif

#ifndef TINY_GSM_MODEM_SIM7672
    if (!modem.setNetworkMode(MODEM_NETWORK_AUTO))
    {
        safePrintln("[Network] setNetworkMode failed");
        return false;
    }
#if DEBUG
    safePrintln("[Network] Network mode set to AUTO");
#endif
#endif

    safePrintln("[Network] Modem initialization completed successfully");
    modemEnabled = true;
    return true;
}

bool Network::disableModem()
{
    if (!modemEnabled)
    {
        safePrintln("[Network] Modem already disabled");
        return true;
    }

    safePrintln("[Network] Disabling modem...");

    if (modem.isGprsConnected())
    {
        modem.gprsDisconnect();
        safePrintln("[Network] GPRS disconnected");
    }

    safePrintln("[Network] Powering down modem...");
    modem.poweroff();

    safePrintln("[Network] Modem disabled successfully");
    modemEnabled = false;
    return true;
}

bool Network::connectLTE(const char* apn)
{
    safePrintln("[Network] Attempting LTE connection...");

    if (!enableModem())
    {
        safePrintln("[Network] Failed to enable modem for LTE connection");
        return false;
    }

    safePrintln("[Network] Checking SIM status...");
    int simRetries = 0;
    while (simRetries < 10 && modem.getSimStatus() != SIM_READY)
    {
#if DEBUG
        safePrintf("[Network] SIM not ready, waiting... attempt %d\n", simRetries + 1);
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
        simRetries++;
    }

    if (modem.getSimStatus() != SIM_READY)
    {
        safePrintln("[Network] SIM card not ready after 10 attempts, disabling modem");
        disableModem();
        return false;
    }
    safePrintln("[Network] SIM card is ready");

#ifdef NETWORK_APN
    safePrintf("[Network] Set network APN : %s\n", NETWORK_APN);
    modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
    if (modem.waitResponse() != 1)
    {
        safePrintln("[Network] Set network APN error !");
    }
#endif

    safePrintln("[Network] Waiting for network registration...");
    int regRetries = 0;
    RegStatus regStatus = REG_UNREGISTERED;
    int16_t sq;

    auto getRegStatusString = [](RegStatus status) -> String
        {
            switch (status)
            {
                case REG_NO_RESULT:
                    return "REG_NO_RESULT (0)";
                case REG_UNREGISTERED:
                    return "REG_UNREGISTERED (1)";
                case REG_SEARCHING:
                    return "REG_SEARCHING (2)";
                case REG_DENIED:
                    return "REG_DENIED (3)";
                case REG_OK_HOME:
                    return "REG_OK_HOME (1)";
                case REG_OK_ROAMING:
                    return "REG_OK_ROAMING (5)";
#ifdef TINY_GSM_MODEM_SIM7672
                case REG_SMS_ONLY:
                    return "REG_SMS_ONLY (6)";
#endif
                default:
                    return "UNKNOWN (" + String(status) + ")";
            }
        };

    // Ok, check network like in lilygo examples
    while (regRetries < 60 && (regStatus != REG_OK_HOME && regStatus != REG_OK_ROAMING))
    {
        regStatus = modem.getRegistrationStatus();
        sq = modem.getSignalQuality();

#if DEBUG
        safePrintf("[Network] Registration status: %s, Signal Quality: %d\n",
            getRegStatusString(regStatus).c_str(), sq);
#endif

        if (regStatus == REG_OK_HOME || regStatus == REG_OK_ROAMING)
        {
            break;
        }
#ifdef TINY_GSM_MODEM_SIM7672
        else if (regStatus == REG_SMS_ONLY)
        {
            safePrintln(
                "[Network] WARNING: Modem registered for SMS only. Data services may be limited.");
            safePrintln("[Network] Continuing with connection attempt...");
            break;
        }
#endif
        else if (regStatus == REG_DENIED)
        {
            safePrintln(
                "[Network] Registration denied by network. Check SIM card and network coverage.");
            disableModem();
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        regRetries++;
    }

    bool registrationOk = false;
    if (regStatus == REG_OK_HOME || regStatus == REG_OK_ROAMING)
    {
        registrationOk = true;
        safePrintln("[Network] Network registration successful");
    }
#ifdef TINY_GSM_MODEM_SIM7672
    else if (regStatus == REG_SMS_ONLY)
    {
        registrationOk = true;
        safePrintln("[Network] Network registration successful (SMS only mode)");
        safePrintln("[Network] Note: Data services may be limited in this mode");
    }
#endif
    else
    {
        safePrintf("[Network] Failed to register to network. Final status: %s\n", getRegStatusString(regStatus).c_str());
        disableModem();
        return false;
    }

    safePrintf("[Network] Connecting to APN: %s\n", apn);

    if (!modem.gprsConnect(apn, "", ""))
    {
        safePrintln("[Network] GPRS connection failed, disabling modem");
        disableModem();
        return false;
    }
    safePrintln("[Network] GPRS connection successful");

    if (!modem.setNetworkActive())
    {
        safePrintln("[Network] Enable network failed!");
    }

#if DEBUG
    String ipAddress = modem.getLocalIP();
    safePrint("Network IP: ");
    safePrintln(ipAddress);
#endif

    bool gprsConnected = modem.isGprsConnected();

    if (gprsConnected)
    {
        safePrintln("[Network] LTE connection established successfully");
        lteConnected = true;
        return true;
    }
    else
    {
        safePrintln("[Network] LTE connection verification failed - GPRS not connected");
        disableModem();
        return false;
    }
}

void Network::disconnectLTE()
{
    safePrintln("[Network] Disconnecting LTE...");
    modem.gprsDisconnect();
    safePrintln("[Network] GPRS disconnected");
    lteConnected = false;
}

bool Network::isLTEConnected()
{
    if (!modemEnabled)
    {
        lteConnected = false;
        return false;
    }

    // Only check gprsconnected, nothing else works, good enough?
    bool result = modem.isGprsConnected();

    lteConnected = result;
    return result;
}

bool Network::isConnected()
{
    return isWiFiConnected() || isLTEConnected();
}

void Network::maintainConnection(const char* ssid, const char* password, const char* apn)
{
    static int16_t wifiScanAttempts = 0;
    static const int8_t MIN_WIFI_ATTEMPTS = 3;
    static uint32_t lastLteAttempt = 0;
    static const uint32_t LTE_RETRY_COOLDOWN = 60000;
    static bool lastWifiState = false;

    bool currentWiFiStatus = isWiFiConnected();

    if (currentWiFiStatus && !lastWifiState)
    {
        safePrintln("[Network] WiFi connection restored");
        wifiScanAttempts = 0;
    }
    lastWifiState = currentWiFiStatus;

    if (currentWiFiStatus)
    {
        wifiScanAttempts = 0;

        if (lteConnected)
        {
            safePrintln("[Network] WiFi connected, disabling LTE modem to save power");
            disconnectLTE();
            disableModem();
            lteConnected = false;
        }
    }
    else
    {
#if DEBUG
        safePrintln("[Network] WiFi not connected, scanning for networks...");
#endif
        int found = WiFi.scanNetworks(false, false, false, 300, 0, ssid);
        bool ssidFound = (found > 0);

        if (ssidFound)
        {
#if DEBUG
            safePrintln("[Network] Target WiFi network found, attempting connection...");
#endif
            bool wifiConnected = connectWiFi(ssid, password);

            if (wifiConnected)
            {
                wifiScanAttempts = 0;

                if (lteConnected)
                {
                    safePrintln("[Network] WiFi connected successfully, disabling LTE modem");
                    disconnectLTE();
                    disableModem();
                    lteConnected = false;
                }
            }
            else
            {
                wifiScanAttempts++;
#if DEBUG
                safePrintf("[Network] WiFi connection failed, attempt %d of %d\n", wifiScanAttempts, MIN_WIFI_ATTEMPTS);
#endif
            }
        }
        else
        {
            wifiScanAttempts++;
#if DEBUG
            safePrintf("[Network] WiFi network not found, scan attempt %d of %d\n", wifiScanAttempts, MIN_WIFI_ATTEMPTS);
#endif
        }

        if (wifiScanAttempts >= MIN_WIFI_ATTEMPTS)
        {
            uint32_t currentTime = millis();

            if (!isLTEConnected())
            {
                if (lastLteAttempt == 0 || (currentTime - lastLteAttempt) >= LTE_RETRY_COOLDOWN)
                {
                    safePrintln(
                        "[Network] Minimum WiFi attempts reached, trying LTE as fallback...");
                    lastLteAttempt = currentTime;
                    bool lteSuccess = connectLTE(apn);

                    if (!lteSuccess)
                    {
                        safePrintln(
                            "[Network] LTE connection failed, will retry after cooldown period");
                    }
                }
                else
                {
#if DEBUG
                    uint32_t timeRemaining = LTE_RETRY_COOLDOWN - (currentTime - lastLteAttempt);
                    safePrintf("[Network] LTE cooldown active, %lu seconds remaining\n", timeRemaining / 1000);
#endif
                }
            }
        }
        else
        {
#if DEBUG
            safePrintln("[Network] Waiting for more WiFi attempts before trying LTE...");
#endif
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
