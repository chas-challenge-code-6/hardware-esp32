#include "network/bluetooth.h"
#include <NimBLEDevice.h>
#include <Arduino.h>

#define WATCH_NAME "ESP32 BLE Emulator" // set to pinetime [unused?]
#define HEARTRATE_SERVICE_UUID "180D"
#define HEARTRATE_CHAR_UUID "2A37"
#define BATTERY_SERVICE_UUID "180F"
#define BATTERY_CHAR_UUID "2A19"
#define ACCEL_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0" //just random uuid
#define ACCEL_CHAR_UUID "12345678-1234-5678-1234-56789abcdef1"
#define STEPS_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" //same
#define STEPS_CHAR_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

const std::string TARGET_ADDRESS = "c4:de:e2:13:3f:0e"; // mac for pinetime watch

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

            NimBLERemoteService *batteryService = pClient->getService(BATTERY_SERVICE_UUID);
            if (batteryService)
            {
                Serial.println("[BluetoothClient] Found battery service");
                NimBLERemoteCharacteristic *batteryChar = batteryService->getCharacteristic(BATTERY_CHAR_UUID);
                if (batteryChar && batteryChar->canNotify())
                {
                    Serial.println("[BluetoothClient] Subscribing to battery notifications");
                    batteryChar->subscribe(true, [this](NimBLERemoteCharacteristic *c, uint8_t *data, size_t len, bool isNotify)
                                           { this->onBatteryNotify(c, data, len, isNotify); });
                }
            }
            else {
                Serial.println("[BluetoothClient] Battery service not found");
            }

            NimBLERemoteService *accelService = pClient->getService(ACCEL_SERVICE_UUID);
            if (accelService)
            {
                Serial.println("[BluetoothClient] Found accel service");
                NimBLERemoteCharacteristic *accelChar = accelService->getCharacteristic(ACCEL_CHAR_UUID);
                if (accelChar && accelChar->canNotify())
                {
                    Serial.println("[BluetoothClient] Subscribing to accel notifications");
                    accelChar->subscribe(true, [this](NimBLERemoteCharacteristic *c, uint8_t *data, size_t len, bool isNotify)
                                         { this->onAccelNotify(c, data, len, isNotify); });
                }
            }
            else {
                Serial.println("[BluetoothClient] Accel service not found");
            }

            NimBLERemoteService *stepsService = pClient->getService(STEPS_SERVICE_UUID);
            if (stepsService)
            {
                Serial.println("[BluetoothClient] Found steps service");
                NimBLERemoteCharacteristic *stepsChar = stepsService->getCharacteristic(STEPS_CHAR_UUID);
                if (stepsChar && stepsChar->canNotify())
                {
                    Serial.println("[BluetoothClient] Subscribing to steps notifications");
                    stepsChar->subscribe(true, [this](NimBLERemoteCharacteristic *c, uint8_t *data, size_t len, bool isNotify)
                                         { this->onStepsNotify(c, data, len, isNotify); });
                }
            }
            else {
                Serial.println("[BluetoothClient] Steps service not found");
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

uint8_t BluetoothClient::getBatteryLevel() const
{
    return batteryLevel;
}

void BluetoothClient::getAccelerometer(float &x, float &y, float &z) const
{
    x = accelX;
    y = accelY;
    z = accelZ;
}

int BluetoothClient::getStepCount() const
{
    return stepCount;
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

void BluetoothClient::onBatteryNotify(NimBLERemoteCharacteristic *, uint8_t *data, size_t len, bool)
{
    if (len > 0)
    {
        batteryLevel = data[0];
        Serial.print("[BluetoothClient] Battery level: ");
        Serial.println(batteryLevel);
    }
}

void BluetoothClient::onAccelNotify(NimBLERemoteCharacteristic *, uint8_t *data, size_t len, bool)
{
    if (len >= 6)
    {
        accelX = (int16_t)((data[1] << 8) | data[0]) / 1000.0f;
        accelY = (int16_t)((data[3] << 8) | data[2]) / 1000.0f;
        accelZ = (int16_t)((data[5] << 8) | data[4]) / 1000.0f;
        Serial.print("[BluetoothClient] Accel: ");
        Serial.print(accelX);
        Serial.print(", ");
        Serial.print(accelY);
        Serial.print(", ");
        Serial.println(accelZ);
    }
}

void BluetoothClient::onStepsNotify(NimBLERemoteCharacteristic *, uint8_t *data, size_t len, bool)
{
    if (len >= 2)
    {
        stepCount = (data[1] << 8) | data[0];
        Serial.print("[BluetoothClient] Steps: ");
        Serial.println(stepCount);
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
