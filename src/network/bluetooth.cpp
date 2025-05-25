#include "network/bluetooth.h"
#include "config.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

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
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new ScanCallbacks(this));
    pScan->setActiveScan(true);
    safePrintln("[BT] Scanning...");
    pScan->start(0, false);
}

void BluetoothClient::loop()
{
    if (doConnect)
    {
        NimBLEClient *pClient = NimBLEDevice::createClient();
        pClient->setClientCallbacks(this, false);

        if (pClient->connect(advDevice))
        {
            NimBLERemoteService *heartRateService = pClient->getService(HEARTRATE_SERVICE_UUID);

            if (heartRateService)
            {
                NimBLERemoteCharacteristic *heartRateChar =
                    heartRateService->getCharacteristic(HEARTRATE_CHAR_UUID);
                if (heartRateChar && heartRateChar->canNotify())
                {
                    safePrintln("[BT] Connected & subscribed");
                    heartRateChar->subscribe(true, [this](NimBLERemoteCharacteristic *c,
                                                          uint8_t *data, size_t len, bool isNotify)
                                             { this->onHeartRateNotify(c, data, len, isNotify); });
                }
            }
            else
            {
                safePrintln("[BT] HR service not found");
            }
        }
        else
        {
            safePrintln("[BT] Connection failed");
        }

        doConnect = false;
        // advDevice = nullptr; // Ta INTE bort advDevice, så vi kan försöka igen vid misslyckad
        // anslutning
    }
}

void BluetoothClient::onConnect(NimBLEClient *pClient)
{
    // meh
}

void BluetoothClient::onDisconnect(NimBLEClient *pClient, int reason)
{
    safePrintf("[BT] Disconnected (reason=%d)\n", reason);
    NimBLEDevice::getScan()->start(0, false);
    // Försök återansluta automatiskt till samma enhet
    if (advDevice)
    {
        doConnect = true;
    }
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
    }
}

void ScanCallbacks::onResult(const NimBLEAdvertisedDevice *advertisedDevice)
{
    std::string advAddr = advertisedDevice->getAddress().toString();
    std::string targetAddr = STRAP_ADDRESS;
    std::transform(advAddr.begin(), advAddr.end(), advAddr.begin(), ::tolower);
    std::transform(targetAddr.begin(), targetAddr.end(), targetAddr.begin(), ::tolower);

    if (advAddr == targetAddr)
    {
        safePrintln("[BT] Target device found");
        client->setConnectFlag(advertisedDevice);
        NimBLEDevice::getScan()->stop();
    }
}
