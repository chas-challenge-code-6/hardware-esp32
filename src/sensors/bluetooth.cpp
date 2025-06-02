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
    setConnectionState(STATE_SCANNING);
}

/**
 * @brief Destroy the Bluetooth Client object
 *
 * @details Properly clean up resources when the object is destroyed
 */
BluetoothClient::~BluetoothClient()
{
#if DEBUG
    safePrintln("[BT] Destroying BluetoothClient...");
#endif

    NimBLEScan* pScan = NimBLEDevice::getScan();
    if (pScan && pScan->isScanning())
    {
        pScan->stop();
    }

    cleanupClient();

    g_btClient = nullptr;
}

/**
 * @brief Set the connect flag for the Bluetooth client
 *
 * @param device The advertised device to connect to
 *
 * @details This function sets the connect flag and safely stores the device address.
 */
void BluetoothClient::setConnectFlag(const NimBLEAdvertisedDevice* device)
{
    if (!device)
    {
        safePrintln("[BT] Error: Null device passed to setConnectFlag");
        return;
    }

    targetDeviceAddress = device->getAddress();
    hasTargetDevice = true;
    doConnect = true;
}

/**
 * @brief Initialize the Bluetooth client
 *
 * @details This function initializes the Bluetooth client and starts scanning for devices.
 */
void BluetoothClient::begin()
{
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(new ScanCallbacks(this));
    pScan->setActiveScan(true);
    pScan->setInterval(80);
    pScan->setWindow(40);
    pScan->setMaxResults(0);

    setConnectionState(STATE_SCANNING);

    safePrintln("[BT] Starting scanning...");
    bool scanStarted = pScan->start(0, false);

    if (!scanStarted)
    {
        safePrintln("[BT] Failed to start scanning, will retry in loop");
    }
    else
    {
#if DEBUG
        safePrintln("[BT] Scanning started successfully");
#endif
    }
}

/**
 * @brief Loop function for the Bluetooth client
 *
 * @details This function uses a state machine to handle connection process with timeout protection
 */
void BluetoothClient::loop()
{
    checkWatchdog();

    uint32_t currentTime = millis();

    if (connectionState != STATE_SCANNING && connectionState != STATE_CONNECTED)
    {
        uint32_t stateTimeout = 0;
        switch (connectionState)
        {
            case STATE_CONNECTING:
                stateTimeout = 10000;
                break;
            case STATE_DISCOVERING_SERVICES:
                stateTimeout = 5000;
                break;
            case STATE_SUBSCRIBING:
                stateTimeout = 3000;
                break;
            default:
                stateTimeout = 8000;
                break;
        }

        if (currentTime - stateStartTime > stateTimeout)
        {
            handleConnectionFailure();
            return;
        }
    }

    switch (connectionState)
    {
        case STATE_SCANNING:
            if (!isScanning())
            {
#if DEBUG
                safePrintln("[BT] Scan stopped unexpectedly, restarting...");
#endif
                restartScanning();
            }

            if (doConnect && hasTargetDevice)
            {
                if (currentTime - lastConnectionAttempt >= CONNECTION_DELAY_MS)
                {
                    startConnectionAttempt();
                }
            }
            break;

        case STATE_CONNECTING:
            break;

        case STATE_DISCOVERING_SERVICES:
            break;

        case STATE_SUBSCRIBING:
            break;

        case STATE_CONNECTED:
            break;

        case STATE_DISCONNECTED:
            if (hasTargetDevice && currentTime - lastConnectionAttempt >= 1000)
            {
                setConnectionState(STATE_SCANNING);
                restartScanning();
                doConnect = true;
            }
            break;
    }
}

/**
 * @brief Set connection state and update timing
 *
 * @param newState The new connection state
 */
void BluetoothClient::setConnectionState(ConnectionState newState)
{
    if (connectionState != newState)
    {
#if DEBUG
        safePrintf("[BT] State: %d -> %d\n", connectionState, newState);
#endif
        connectionState = newState;
        stateStartTime = millis();
    }
}

/**
 * @brief Start a connection attempt with state management
 */
void BluetoothClient::startConnectionAttempt()
{
    if (!hasTargetDevice)
    {
#if DEBUG
        safePrintln("[BT] No target device stored, clearing connect flag");
#endif
        doConnect = false;
        return;
    }

    lastConnectionAttempt = millis();
    connectionAttempts++;

    if (connectionAttempts > MAX_CONNECTION_ATTEMPTS)
    {
        safePrintln("[BT] Max connection attempts reached, restarting scan");
        handleConnectionFailure();
        return;
    }

    safePrintf("[BT] Connection attempt %d/%d to %s\n",
        connectionAttempts, MAX_CONNECTION_ATTEMPTS,
        targetDeviceAddress.toString().c_str());

    cleanupClient();

    setConnectionState(STATE_CONNECTING);

    pClient = NimBLEDevice::createClient();
    if (!pClient)
    {
        safePrintln("[BT] Failed to create client");
        handleConnectionFailure();
        return;
    }

    pClient->setClientCallbacks(this, false);
    //pClient->setConnectionParams(16, 32, 0, 400); // default is good enough
    pClient->setConnectTimeout(2 * 3000); // timeout efter 3s

#if DEBUG
    safePrintln("[BT] Starting async connection...");
#endif

    bool connectResult = pClient->connect(targetDeviceAddress);

    if (!connectResult)
    {
        safePrintln("[BT] Immediate connection failure");
        handleConnectionFailure();
    }
}

/**
 * @brief Handle connection failure and reset state
 */
void BluetoothClient::handleConnectionFailure()
{
#if DEBUG
    safePrintln("[BT] Handling connection failure");
#endif

    cleanupClient();
    doConnect = false;

    if (connectionAttempts >= MAX_CONNECTION_ATTEMPTS)
    {
        safePrintln("[BT] Max attempts reached, clearing target device");
        connectionAttempts = 0;
        hasTargetDevice = false;
        setConnectionState(STATE_SCANNING);
        restartScanning();
    }
    else
    {
#if DEBUG
        safePrintln("[BT] Will retry connection");
#endif
        setConnectionState(STATE_DISCONNECTED);
        lastConnectionAttempt = millis();
    }
}

/**
 * @brief Discover services with timeout protection
 */
void BluetoothClient::discoverServices()
{
    setConnectionState(STATE_DISCOVERING_SERVICES);

#if DEBUG
    safePrintln("[BT] Starting service discovery...");
#endif

    vTaskDelay(pdMS_TO_TICKS(50));

    NimBLERemoteService* heartRateService = pClient->getService(HEARTRATE_SERVICE_UUID);

    if (heartRateService)
    {
#if DEBUG
        safePrintln("[BT] Heart rate service found");
#endif

        vTaskDelay(pdMS_TO_TICKS(50));

        NimBLERemoteCharacteristic* heartRateChar =
            heartRateService->getCharacteristic(HEARTRATE_CHAR_UUID);

        if (heartRateChar && heartRateChar->canNotify())
        {
#if DEBUG
            safePrintln("[BT] Heart rate characteristic found, subscribing...");
#endif
            subscribeToCharacteristic(heartRateChar);
        }
        else
        {
            safePrintln("[BT] HR characteristic not found or can't notify");
            handleConnectionFailure();
        }
    }
    else
    {
        safePrintln("[BT] HR service not found");
        handleConnectionFailure();
    }
}

/**
 * @brief Subscribe to characteristic with timeout protection
 */
void BluetoothClient::subscribeToCharacteristic(NimBLERemoteCharacteristic* characteristic)
{
    setConnectionState(STATE_SUBSCRIBING);

    vTaskDelay(pdMS_TO_TICKS(50));

    bool subscribeResult = characteristic->subscribe(true, [this](NimBLERemoteCharacteristic* c,
        uint8_t* data, size_t len, bool isNotify)
        {
            this->onHeartRateNotify(c, data, len, isNotify);
        });

    if (subscribeResult)
    {
        safePrintln("[BT] Successfully subscribed to heart rate notifications");
        connectionAttempts = 0;
        lastHeartRateUpdate = millis();
        setConnectionState(STATE_CONNECTED);
    }
    else
    {
        safePrintln("[BT] Failed to subscribe to notifications");
        handleConnectionFailure();
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
    safePrintln("[BT] Connected callback triggered");

    if (connectionState == STATE_CONNECTING)
    {
        discoverServices();
    }
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

    setConnectionState(STATE_DISCONNECTED);
    lastConnectionAttempt = millis();

    if (hasTargetDevice)
    {
        doConnect = true;
    }
}

/**
 * @brief Clean up the current client connection
 *
 * @details Safely disconnects and deletes the current client
 */
void BluetoothClient::cleanupClient()
{
    if (pClient)
    {
        if (pClient->isConnected())
        {
            pClient->disconnect();
        }
        NimBLEDevice::deleteClient(pClient);
        pClient = nullptr;
    }
}

/**
 * @brief Check if currently connected to a device
 *
 * @return true if connected, false otherwise
 */
bool BluetoothClient::isConnected() const
{
    return pClient && pClient->isConnected();
}

/**
 * @brief Check if currently scanning for devices
 *
 * @return true if scanning, false otherwise
 */
bool BluetoothClient::isScanning() const
{
    NimBLEScan* pScan = NimBLEDevice::getScan();
    return pScan && pScan->isScanning();
}

/**
 * @brief Restart the scanning process
 *
 * @details Safely stops and restarts scanning
 */
void BluetoothClient::restartScanning()
{
    NimBLEScan* pScan = NimBLEDevice::getScan();
    if (pScan)
    {
        if (pScan->isScanning())
        {
            pScan->stop();
            vTaskDelay(pdMS_TO_TICKS(50));
        }

#if DEBUG
        safePrintln("[BT] Restarting scan...");
#endif
        bool scanStarted = pScan->start(0, false);

        if (!scanStarted)
        {
            safePrintln("[BT] Failed to restart scanning");
        }
        else
        {
#if DEBUG
            safePrintln("[BT] Scan restarted successfully");
#endif
            setConnectionState(STATE_SCANNING);
        }
    }
}

/**
 * @brief Check watchdog conditions and handle timeout scenarios
 *
 * @details Monitors heart rate updates and scan status for hangs
 */
void BluetoothClient::checkWatchdog()
{
    uint32_t currentTime = millis();

    // Check heart rate timeout only when connected
    if (connectionState == STATE_CONNECTED &&
        (currentTime - lastHeartRateUpdate > HEARTRATE_TIMEOUT_MS))
    {
        safePrintln("[BT] Watchdog: Heart rate timeout, reconnecting");
        handleConnectionFailure();
        return;
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
    // returnera 0 om felaktigt värde
    return (heartRate == 255) ? 0 : heartRate;
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
        if ((flags & 0x01) == 0)
        {
            heartRate = data[1];
        }
        else
        {
            heartRate = (data[2] << 8) | data[1];
        }

        lastHeartRateUpdate = millis();
    }
}

/**
 * @brief Callback function for scan results
 *
 * @param advertisedDevice The advertised device found during scanning
 *
 * @details This function is called when a scan result is found.
 */
void ScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice)
{
    if (!advertisedDevice) return;

    std::string advAddr = advertisedDevice->getAddress().toString();
    std::string targetAddr = STRAP_ADDRESS;

    std::transform(advAddr.begin(), advAddr.end(), advAddr.begin(), ::tolower);
    std::transform(targetAddr.begin(), targetAddr.end(), targetAddr.begin(), ::tolower);

    if (advAddr == targetAddr)
    {
        safePrint("[BT] Target device found: ");
        safePrintln(advAddr.c_str());

        NimBLEDevice::getScan()->stop();
        
        client->setConnectFlag(advertisedDevice);
    }
}
