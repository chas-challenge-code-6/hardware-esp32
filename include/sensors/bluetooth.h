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

/**
 * @brief Class to handle Bluetooth client operations
 * 
 */
class BluetoothClient : public NimBLEClientCallbacks
{
public:
    enum ConnectionState {
        STATE_SCANNING,
        STATE_CONNECTING,
        STATE_DISCOVERING_SERVICES,
        STATE_SUBSCRIBING,
        STATE_CONNECTED,
        STATE_DISCONNECTED
    };

    BluetoothClient();
    ~BluetoothClient();
    void begin();
    void loop();

    uint8_t getHeartRate() const;

    void onConnect(NimBLEClient *pClient) override;
    void onDisconnect(NimBLEClient *pClient, int reason) override;
    void setConnectFlag(const NimBLEAdvertisedDevice *device);

private:
    void onHeartRateNotify(NimBLERemoteCharacteristic *, uint8_t *, size_t, bool);
    void cleanupClient();
    void checkWatchdog();
    bool isConnected() const;
    bool isScanning() const;
    void restartScanning();
    
    void setConnectionState(ConnectionState newState);
    void startConnectionAttempt();
    void handleConnectionFailure();
    void discoverServices();
    void subscribeToCharacteristic(NimBLERemoteCharacteristic* characteristic);

    uint8_t heartRate = 255;
    bool doConnect = false;
    NimBLEAddress targetDeviceAddress;
    bool hasTargetDevice = false;
    
    NimBLEClient* pClient = nullptr;
    
    uint32_t lastHeartRateUpdate = 0;
    uint32_t lastConnectionAttempt = 0;
    uint8_t connectionAttempts = 0;

    // configuration
    static const uint8_t MAX_CONNECTION_ATTEMPTS = 3;
    static const uint32_t CONNECTION_DELAY_MS = 3000;
    static const uint32_t HEARTRATE_TIMEOUT_MS = 30000;  // 30 seconds is enough
    
    ConnectionState connectionState = STATE_SCANNING;
    uint32_t stateStartTime = 0;
};

/**
 * @brief Class to handle Bluetooth scan callbacks
 * 
 */
class ScanCallbacks : public NimBLEScanCallbacks
{
public:
    ScanCallbacks(BluetoothClient *client) : client(client) {}
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;

private:
    BluetoothClient *client;
};

#endif
