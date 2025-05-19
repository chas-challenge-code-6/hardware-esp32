#include "network/bluetooth.h"
#include "main.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

const std::string TARGET_ADDRESS = "a0:9e:1a:ec:35:1e";

static BluetoothClient *g_btClient = nullptr;

BluetoothClient::BluetoothClient()
{
    g_btClient = this;
}

void BluetoothClient::setConnectFlag(const NimBLEAdvertisedDevice *device)
{
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
    pScan->start(0, false);
}

void BluetoothClient::loop()
{
    if (doConnect)
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
                NimBLERemoteCharacteristic *heartRateChar =
                    heartRateService->getCharacteristic(HEARTRATE_CHAR_UUID);
                if (heartRateChar && heartRateChar->canNotify())
                {
                    Serial.println("[BluetoothClient] Subscribing to heart rate notifications");
                    heartRateChar->subscribe(true, [this](NimBLERemoteCharacteristic *c,
                                                          uint8_t *data, size_t len, bool isNotify)
                                             { this->onHeartRateNotify(c, data, len, isNotify); });
                }
            }
            else
            {
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

void BluetoothClient::onHeartRateNotify(NimBLERemoteCharacteristic *, uint8_t *data, size_t len,
                                        bool)
{
    if (len > 0)
    {
        uint8_t flags = data[0];
        if ((flags & 0x01) == 0)
        {
            heartRate = data[1];
        }
        else
        {
            heartRate = (data[2] << 8) | data[1];
        }
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
    std::string targetAddr = TARGET_ADDRESS;
    std::transform(advAddr.begin(), advAddr.end(), advAddr.begin(), ::tolower);
    std::transform(targetAddr.begin(), targetAddr.end(), targetAddr.begin(), ::tolower);

    if (advAddr == targetAddr)
    {
        Serial.println("[ScanCallbacks] Target device address matched!");
        client->setConnectFlag(advertisedDevice);
        NimBLEDevice::getScan()->stop();
    }
}
