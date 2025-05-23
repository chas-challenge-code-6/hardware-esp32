/**
 * @file bluetooth.cpp
 * @brief Bluetooth Client Class
 * 
 * @details This file contains the implementation of the BluetoothClient class, which is used to manage
 * Bluetooth connections and notifications.
 */


#include "network/bluetooth.h"
#include "main.h"
#include <Arduino.h>
#include <NimBLEDevice.h>



const std::string TARGET_ADDRESS = "a0:9e:1a:ec:35:1e"; /// Target device MAC address

static BluetoothClient *g_btClient = nullptr; ///< UUIDs for the heart rate service and characteristic


/**
 * @brief Construct a new Bluetooth Client object
 * 
 */
BluetoothClient::BluetoothClient()
{
    g_btClient = this;
}

/**
 * @brief Set the connect flag for the Bluetooth client
 * 
 * @param device The advertised device to connect to
 * 
 * @details This function sets the connect flag and stores the advertised device.
 */
void BluetoothClient::setConnectFlag(const NimBLEAdvertisedDevice *device)
{
    doConnect = true;
    advDevice = device;
}


/**
 * @brief Initialize the Bluetooth client
 * 
 * @details This function initializes the Bluetooth client and starts scanning for devices.
 */
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


/**
 * @brief Loop function for the Bluetooth client
 * 
 * @details This function checks if a connection is needed and handles the connection process.
 */
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


/**
 * @brief Callback function for when the client connects to a device
 * 
 * @param pClient The connected client
 * 
 * @details This function is called when the client successfully connects to a device.
 */
void BluetoothClient::onConnect(NimBLEClient *pClient)
{
    Serial.println("[BluetoothClient] onConnect: Connected to server");
}


/**
 * @brief Callback function for when the client disconnects from a device
 * 
 * @param pClient The disconnected client
 * @param reason The reason for disconnection
 * 
 * @details This function is called when the client disconnects from a device.
 */
void BluetoothClient::onDisconnect(NimBLEClient *pClient, int reason)
{
    Serial.printf("[BluetoothClient] onDisconnect: Disconnected, reason=%d\n", reason);
    NimBLEDevice::getScan()->start(0, false);
}


/**
 * @brief Get the heart rate value
 * 
 * @return uint8_t The heart rate value
 * 
 * @details This function returns the current heart rate value.
 */
uint8_t BluetoothClient::getHeartRate() const
{
    return heartRate;
}


/**
 * @brief Callback function for heart rate notifications
 * 
 * @param c The characteristic that sent the notification
 * @param data The notification data
 * @param len The length of the notification data
 * @param isNotify Whether the notification is a notify or indicate
 * 
 * @details This function is called when a heart rate notification is received.
 */
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


/**
 * @brief Callback function for scan results
 * 
 * @param advertisedDevice The advertised device found during scanning
 * 
 * @details This function is called when a scan result is found.
 */
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
