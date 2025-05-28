/**
 /**
 * @file bluetooth.cpp
 * @brief Bluetooth Client Class
 *
 * @details This file contains the implementation of the BluetoothClient class, which is used to
 * manage Bluetooth connections and notifications.
 */

#include "sensors/bluetooth.h"
#include "config.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

static BluetoothClient* g_btClient = nullptr;

/**
 * @brief Construct a new Bluetooth Client object
 *
 */
BluetoothClient::BluetoothClient()
{
    g_btClient = this;
}

/**
 * @brief Destroy the Bluetooth Client object
 *
 * @details Properly clean up resources when the object is destroyed
 */
BluetoothClient::~BluetoothClient()
{
    safePrintln("[BT] Destroying BluetoothClient...");

    NimBLEScan* pScan = NimBLEDevice::getScan();
    if (pScan && pScan->isScanning())
    {
        pScan->stop();
    }

    cleanupClient();

    if (scanCallbacks != nullptr)
    {
        delete scanCallbacks;
        scanCallbacks = nullptr;
    }

    g_btClient = nullptr;

    safePrintln("[BT] BluetoothClient destroyed");
}

/**
 * @brief Set the connect flag for the Bluetooth client
 *
 * @param device The advertised device to connect to
 *
 * @details This function sets the connect flag and stores the advertised device.
 */
void BluetoothClient::setConnectFlag(const NimBLEAdvertisedDevice* device)
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
    safePrintln("[BT] Initializing Bluetooth...");

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    scanCallbacks = new ScanCallbacks(this);

    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(scanCallbacks);
    pScan->setActiveScan(true);
    pScan->setInterval(100);
    pScan->setWindow(99);
    pScan->setMaxResults(0);

    safePrintln("[BT] Starting scanning...");
    bool scanStarted = pScan->start(0, false);

    if (!scanStarted)
    {
        safePrintln("[BT] Failed to start scanning, will retry in loop");
    }
    else
    {
        safePrintln("[BT] Scanning started successfully");
    }
}

/**
 * @brief Loop function for the Bluetooth client
 *
 * @details This function checks if a connection is needed and handles the connection process.
 */
void BluetoothClient::loop()
{
    checkWatchdog();

    if (!doConnect && !isConnected() && !isScanning())
    {
        safePrintln("[BT] Scan stopped unexpectedly, restarting...");
        restartScanning();
    }

    if (doConnect)
    {
        if (millis() - lastConnectionAttempt < 10000)
        {
            return;
        }

        lastConnectionAttempt = millis();
        connectionAttempts++;

        if (connectionAttempts > MAX_CONNECTION_ATTEMPTS)
        {
            safePrintln("[BT] Max connection attempts reached, restarting scan");
            connectionAttempts = 0;
            doConnect = false;
            restartScanning();
            return;
        }

        safePrintf("[BT] Connection attempt %d/%d\n", connectionAttempts, MAX_CONNECTION_ATTEMPTS);

        cleanupClient();

        pClient = NimBLEDevice::createClient();
        if (!pClient)
        {
            safePrintln("[BT] Failed to create client");
            return;
        }

        pClient->setClientCallbacks(this, false);
        pClient->setConnectionParams(6, 24, 0, 512);
        pClient->setConnectTimeout(CONNECTION_TIMEOUT_MS / 1000);

        safePrintln("[BT] Attempting to connect...");
        if (pClient->connect(advDevice))
        {
            safePrintln("[BT] Connected to device");
            connectionAttempts = 0;
            lastHeartRateUpdate = millis();

            NimBLERemoteService* heartRateService = pClient->getService(HEARTRATE_SERVICE_UUID);

            if (heartRateService)
            {
                NimBLERemoteCharacteristic* heartRateChar =
                    heartRateService->getCharacteristic(HEARTRATE_CHAR_UUID);
                if (heartRateChar && heartRateChar->canNotify())
                {
                    safePrintln("[BT] Connected & subscribed");
                    heartRateChar->subscribe(true, [this](NimBLERemoteCharacteristic* c,
                        uint8_t* data, size_t len, bool isNotify)
                        {
                            this->onHeartRateNotify(c, data, len, isNotify);
                        });
                }
                else
                {
                    safePrintln("[BT] HR characteristic not found or can't notify");
                    cleanupClient();
                }
            }
            else
            {
                safePrintln("[BT] HR service not found");
                cleanupClient();
            }
        }
        else
        {
            safePrintln("[BT] Connection failed");
            cleanupClient();
        }

        doConnect = false;
    }
}

/**
 * @brief Callback function for when the client connects to a device
 *
 * @param pClient The connected client
 *
 * @details This function is called when the client successfully connects to a device.
 */
void BluetoothClient::onConnect(NimBLEClient* pClient)
{
    // meh
}

/**
 * @brief Callback function for when the client disconnects from a device
 *
 * @param pClient The disconnected client
 * @param reason The reason for disconnection
 *
 * @details This function is called when the client disconnects from a device.
 */
void BluetoothClient::onDisconnect(NimBLEClient* pClient, int reason)
{
    safePrintf("[BT] Disconnected (reason=%d)\n", reason);

    connectionAttempts = 0;

    cleanupClient();

    vTaskDelay(pdMS_TO_TICKS(1000));

    restartScanning();

    if (advDevice)
    {
        doConnect = true;
        lastConnectionAttempt = millis();
    }
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
    return (heartRate == 255) ? 0 : heartRate; // skicka 0 om 255
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
void BluetoothClient::onHeartRateNotify(NimBLERemoteCharacteristic*, uint8_t* data, size_t len,
    bool)
{
    if (len > 0)
    {
        uint8_t flags = data[0];
        uint16_t newHeartRate = 0;

        if ((flags & 0x01) == 0)
        {
            newHeartRate = data[1];
        }
        else
        {
            newHeartRate = (data[2] << 8) | data[1];
        }

        if (newHeartRate >= 10 && newHeartRate <= 250)
        {
            heartRate = newHeartRate;
            lastHeartRateUpdate = millis();
            safePrint("[BT] Heart rate: ");
            safePrintln(String(heartRate).c_str());
        }
        else
        {
            safePrint("[BT] Invalid heart rate data: ");
            safePrintln(String(newHeartRate).c_str());
        }
    }
}

/**
 * @brief Callback function for scan results
 *
 * @param advertisedDevice The advertised device found during scanning
 *
 * @details This function is called when a scan result is found.
 */
 /**
  * @brief Clean up the current BLE client connection
  *
  * @details This function properly disconnects and deletes the current client
  * to prevent memory leaks and connection state issues.
  */
void BluetoothClient::cleanupClient()
{
    if (pClient != nullptr)
    {
        if (pClient->isConnected())
        {
            safePrintln("[BT] Disconnecting client...");
            pClient->disconnect();
        }

        // Delete the client to free memory
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
        safePrintln("[BT] Client cleaned up");
    }
}

/**
 * @brief Restart Bluetooth scanning
 *
 * @details This function stops the current scan and starts a new one
 * to recover from scanning issues.
 */
void BluetoothClient::restartScanning()
{
    safePrintln("[BT] Restarting scanning...");

    NimBLEScan* pScan = NimBLEDevice::getScan();

    if (pScan->isScanning())
    {
        pScan->stop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    pScan->clearResults();

    pScan->start(0, false);
    safePrintln("[BT] Scanning restarted");
}

/**
 * @brief Check if client is currently connected
 */
bool BluetoothClient::isConnected() const
{
    return (pClient != nullptr && pClient->isConnected());
}

/**
 * @brief Check if scanning is currently active
 */
bool BluetoothClient::isScanning() const
{
    NimBLEScan* pScan = NimBLEDevice::getScan();
    return (pScan != nullptr && pScan->isScanning());
}

/**
 * @brief Force restart of entire Bluetooth subsystem
 */
void BluetoothClient::forceRestart()
{
    safePrintln("[BT] Force restarting Bluetooth subsystem...");

    cleanupClient();

    NimBLEScan* pScan = NimBLEDevice::getScan();
    if (pScan && pScan->isScanning())
    {
        pScan->stop();
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    doConnect = false;
    connectionAttempts = 0;
    heartRate = 255;
    lastConnectionAttempt = 0;
    lastHeartRateUpdate = millis();
    lastScanCheck = millis();

    restartScanning();

    safePrintln("[BT] Bluetooth subsystem restarted");
}

/**
 * @brief Watchdog function to detect and recover from stuck states
 */
void BluetoothClient::checkWatchdog()
{
    uint32_t currentTime = millis();

    if (isConnected() && lastHeartRateUpdate > 0 &&
        (currentTime - lastHeartRateUpdate) > HEARTRATE_TIMEOUT_MS)
    {
        safePrintln("[BT] Watchdog: No heart rate data for too long, forcing restart");
        forceRestart();
        return;
    }

    if (!doConnect && !isConnected() &&
        (currentTime - lastScanCheck) > SCAN_CHECK_INTERVAL_MS)
    {
        lastScanCheck = currentTime;
        if (!isScanning())
        {
            safePrintln("[BT] Watchdog: Scanning stopped, restarting");
            restartScanning();
        }
    }
}

void ScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice)
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
