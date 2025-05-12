/* 
This file is for the Polar HRT Bluetooth client implementation.


*/


#include "network/bluetooth.h"
#include <NimBLEDevice.h>
#include <Arduino.h>

#define WATCH_NAME "PolarHRT" // set to pinetime [unused?]
#define HEARTRATE_SERVICE_UUID "180D"
#define HEARTRATE_CHAR_UUID "2A37"


const std::string TARGET_ADDRESS = "Adding later"; // mac for pinetime watch

static BluetoothClient *g_btClient = nullptr;

BluetoothClient::BluetoothClient()
{
    g_btClient = this;
}

void BluetoothClient::setConnectFlag(const NimBLEAdvertisedDevice* device) {
    doConnect = true;
    advDevice = device;
}

void BluetoothClient::begin()
{
    Serial.println("[BluetoothClient] begin() called");
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new ScanCallbacks(this));
    pScan->setActiveScan(true);
    Serial.println("[BluetoothClient] NimBLE initialized, starting scan...");
    pScan->start(0, false); // Start scanning immediately
}

void BluetoothClient::loop()
{
    if (doConnect && advDevice)
    {
        Serial.println("[BluetoothClient] Connecting to device...");
        NimBLEClient *pClient = NimBLEDevice::createClient();
        pClient->setClientCallbacks(this, false);
        if (pClient->connect(advDevice))
        {
            Serial.println("[BluetoothClient] Connected to device");
            NimBLERemoteService *heartRateService = pClient->getService(HEARTRATE_SERVICE_UUID);
            if (heartRateService)
            {
                Serial.println("[BluetoothClient] Found heartrate service");
                NimBLERemoteCharacteristic *heartRateChar = heartRateService->getCharacteristic(HEARTRATE_CHAR_UUID);
                if (heartRateChar && heartRateChar->canNotify())
                {
                    Serial.println("[BluetoothClient] Subscribing to heart rate notifications");
                    heartRateChar->subscribe(true, [this](NimBLERemoteCharacteristic *c, uint8_t *data, size_t len, bool isNotify)
                                           { this->onHeartRateNotify(c, data, len, isNotify); });
                }
            }
            else {
                Serial.println("[BluetoothClient] Heartrate service not found");
            }

        }
        else
        {
            Serial.println("[BluetoothClient] Failed to connect");
        }
        doConnect = false;
        advDevice = nullptr;
    }
}

void BluetoothClient::onConnect(NimBLEClient *pClient)
{
    Serial.println("[BluetoothClient] onConnect: Connected to server");
}

void BluetoothClient::onDisconnect(NimBLEClient *pClient, int reason)
{
    Serial.printf("[BluetoothClient] onDisconnect: Disconnected, reason=%d\n", reason);
    NimBLEDevice::getScan()->start(0, false);
}

uint8_t BluetoothClient::getHeartRate() const
{
    return heartRate;
}



void BluetoothClient::onHeartRateNotify(NimBLERemoteCharacteristic *, uint8_t *data, size_t len, bool)
{
    if (len > 0)
    {
        heartRate = data[1];
        Serial.print("[BluetoothClient] Heart rate: ");
        Serial.println(heartRate);
    }
}


void ScanCallbacks::onResult(const NimBLEAdvertisedDevice *advertisedDevice)
{
    Serial.print("[ScanCallbacks] Found device: ");
    Serial.println(advertisedDevice->toString().c_str());
    Serial.print("[ScanCallbacks] Device address: ");
    Serial.println(advertisedDevice->getAddress().toString().c_str());
    std::string advAddr = advertisedDevice->getAddress().toString();
    std::transform(advAddr.begin(), advAddr.end(), advAddr.begin(), ::tolower);
    std::string targetAddr = TARGET_ADDRESS;
    std::transform(targetAddr.begin(), targetAddr.end(), targetAddr.begin(), ::tolower);
    if (advAddr == targetAddr)
    {
        Serial.println("[ScanCallbacks] Target device address matched!");
        client->setConnectFlag(advertisedDevice);
        NimBLEDevice::getScan()->stop();
    }
}
