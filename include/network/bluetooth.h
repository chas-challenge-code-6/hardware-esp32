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

class BluetoothClient : public NimBLEClientCallbacks {
public:
    BluetoothClient();
    void begin();
    void loop();

    uint8_t getHeartRate() const;

    void onConnect(NimBLEClient* pClient) override;
    void onDisconnect(NimBLEClient* pClient, int reason) override;

    void setConnectFlag(const NimBLEAdvertisedDevice* device);

private:
    void onHeartRateNotify(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool);

    uint8_t heartRate = -1;
    bool doConnect = false;
    const NimBLEAdvertisedDevice* advDevice = nullptr;
};


/**
 * @brief Class to handle Bluetooth scan callbacks
 * 
 */

class ScanCallbacks : public NimBLEScanCallbacks {
public:
    ScanCallbacks(BluetoothClient* client) : client(client) {}
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;

private:
    BluetoothClient* client;
};

#endif