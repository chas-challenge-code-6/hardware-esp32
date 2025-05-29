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

bool Network::enableModem()
{
    if (modemEnabled)
    {
        safePrintln("[Network] Modem already enabled");
        return true;
    }

    safePrintln("[Network] Initializing modem hardware...");

#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
    safePrintln("[Network] Modem power enabled");
#endif

#ifdef MODEM_RESET_PIN
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(10));
    digitalWrite(MODEM_RESET_PIN, HIGH);
    safePrintln("[Network] Modem reset sequence completed");
#endif

    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    safePrintln("[Network] Modem power key sequence completed");

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
    safePrintln("[Network] Modem serial initialized at 115200 baud");

    vTaskDelay(pdMS_TO_TICKS(3000));

    safePrintln("[Network] Testing modem responsiveness...");
    int attempts = 0;
    while (attempts < 3 && !modem.testAT(1000))
    {
        safePrint("[Network] Modem not responding, attempt ");
        safePrintln(String(attempts + 1));
        attempts++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (attempts >= 3)
    {
        safePrintln("[Network] Modem failed to respond to AT commands");
        return false;
    }
    safePrintln("[Network] Modem is responding to AT commands");

    safePrintln("[Network] Initializing modem...");
    if (!modem.init())
    {
        safePrintln("[Network] Failed to initialize modem");
        return false;
    }

    String name = modem.getModemName();
    String info = modem.getModemInfo();
    safePrint("[Network] Modem Name: ");
    safePrintln(name);
    safePrint("[Network] Modem Info: ");
    safePrintln(info);

#ifndef TINY_GSM_MODEM_SIM7672 // if using sentinel-fredrik
    if (!modem.setNetworkMode(MODEM_NETWORK_AUTO))
    {
        safePrintln("[Network] setNetworkMode failed");
        return false;
    }
    safePrintln("[Network] Network mode set to AUTO");
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

bool Network::connectLTE(const char *apn)
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
        safePrint("[Network] SIM not ready, waiting... attempt ");
        safePrintln(String(simRetries + 1));
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

    while (regRetries < 60 && (regStatus != REG_OK_HOME && regStatus != REG_OK_ROAMING))
    {
        regStatus = modem.getRegistrationStatus();

        safePrint("[Network] Registration status: ");
        safePrintln(getRegStatusString(regStatus));

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
        safePrint("[Network] Failed to register to network. Final status: ");
        safePrintln(getRegStatusString(regStatus));
        disableModem();
        return false;
    }

    safePrint("[Network] Connecting to APN: ");
    safePrintln(apn);

    /*
    if (!modem.gprsConnect(apn, "", ""))
    {
        safePrintln("[Network] GPRS connection failed, disabling modem");
        disableModem();
        return false;
    }
    safePrintln("[Network] GPRS connection successful");
    */

    if (!modem.setNetworkActive())
    {
        safePrintln("[Network] Enable network failed!");
    }

    String ipAddress = modem.getLocalIP();
    safePrint("Network IP: ");
    safePrintln(ipAddress);

    // if (modem.isNetworkConnected() && modem.isGprsConnected())
    if (modem.isGprsConnected())
    // if (modem.isNetworkConnected())
    {
        safePrintln("[Network] LTE connection established successfully");
        lteConnected = true;
        return true;
    }
    else
    {
        safePrintln("[Network] LTE connection verification failed, disabling modem");
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

    // bool result = modem.isNetworkConnected() && modem.isGprsConnected();
    bool result = modem.isGprsConnected();
    // bool result = modem.isNetworkConnected();
    lteConnected = result;
    return result;
}

bool Network::isConnected()
{
    return isWiFiConnected() || isLTEConnected();
}

void Network::maintainConnection(const char *ssid, const char *password, const char *apn)
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
        safePrintln("[Network] WiFi not connected, scanning for networks...");
        int found = WiFi.scanNetworks(false, false, false, 300, 0, ssid);
        bool ssidFound = (found > 0);

        if (ssidFound)
        {
            safePrintln("[Network] Target WiFi network found, attempting connection...");
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
                safePrint("[Network] WiFi connection failed, attempt ");
                safePrint(String(wifiScanAttempts));
                safePrint(" of ");
                safePrintln(String(MIN_WIFI_ATTEMPTS));
            }
        }
        else
        {
            wifiScanAttempts++;
            safePrint("[Network] WiFi network not found, scan attempt ");
            safePrint(String(wifiScanAttempts));
            safePrint(" of ");
            safePrintln(String(MIN_WIFI_ATTEMPTS));
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
                    uint32_t timeRemaining = LTE_RETRY_COOLDOWN - (currentTime - lastLteAttempt);
                    safePrint("[Network] LTE cooldown active, ");
                    safePrint(String(timeRemaining / 1000));
                    safePrintln(" seconds remaining");
                }
            }
        }
        else
        {
            safePrintln("[Network] Waiting for more WiFi attempts before trying LTE...");
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
