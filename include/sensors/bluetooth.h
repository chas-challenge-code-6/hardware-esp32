/**
 /**
 * @file bluetooth.h
 * @brief Bluetooth Client Class
 *
 * @details This file contains the declaration of the BluetoothClient class, which is used to manage
 *
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <NimBLEDevice.h>

class ScanCallbacks;

/**
 * @brief Class to handle Bluetooth client operations
 *
 */
class BluetoothClient : public NimBLEClientCallbacks
{
public:
    BluetoothClient();
    ~BluetoothClient();
    void begin();
    void loop();

    uint8_t getHeartRate() const;
    bool isConnected() const;
    bool isScanning() const;
    void forceRestart();

    void onConnect(NimBLEClient* pClient) override;
    void onDisconnect(NimBLEClient* pClient, int reason) override;
    void setConnectFlag(const NimBLEAdvertisedDevice* device);

private:
    void onHeartRateNotify(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool);
    void cleanupClient();
    void restartScanning();
    void checkWatchdog();

    uint8_t heartRate = 255;
    bool doConnect = false;
    const NimBLEAdvertisedDevice* advDevice = nullptr;
    NimBLEClient* pClient = nullptr;
    ScanCallbacks* scanCallbacks = nullptr;
    uint32_t lastConnectionAttempt = 0;
    uint32_t lastHeartRateUpdate = 0;
    uint32_t lastScanCheck = 0;
    int connectionAttempts = 0;
    static const int16_t MAX_CONNECTION_ATTEMPTS = 5;
    static const uint32_t CONNECTION_TIMEOUT_MS = 15000;
    static const uint32_t HEARTRATE_TIMEOUT_MS = 60000;
    static const uint32_t SCAN_CHECK_INTERVAL_MS = 30000;
};

/**
 * @brief Class to handle Bluetooth scan callbacks
 *
 */
class ScanCallbacks : public NimBLEScanCallbacks
{
public:
    ScanCallbacks(BluetoothClient* client) : client(client) {}
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;

private:
    BluetoothClient* client;
};

#endif
