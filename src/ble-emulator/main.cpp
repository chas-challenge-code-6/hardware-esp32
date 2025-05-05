#include <Arduino.h>
#include <NimBLEDevice.h>

#define SERVICE_UUID_BATTERY       "180F"
#define SERVICE_UUID_HEART_RATE    "180D"
#define SERVICE_UUID_STEP_COUNTER  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" //TEMP

#define CHAR_UUID_BATTERY_LEVEL    "2A19"
#define CHAR_UUID_HEART_RATE       "2A37"
#define CHAR_UUID_STEP_COUNT       "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" //TEMP

NimBLECharacteristic *batteryLevelChar;
NimBLECharacteristic *heartRateChar;
NimBLECharacteristic *stepCountChar;

void setup() {
  Serial.begin(115200);
  NimBLEDevice::init("ESP32 BLE Emulator");

  NimBLEServer *pServer = NimBLEDevice::createServer();

  NimBLEService *batteryService = pServer->createService(SERVICE_UUID_BATTERY);
  batteryLevelChar = batteryService->createCharacteristic(
    CHAR_UUID_BATTERY_LEVEL,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  batteryLevelChar->setValue(100);

  NimBLEService *heartRateService = pServer->createService(SERVICE_UUID_HEART_RATE);
  heartRateChar = heartRateService->createCharacteristic(
    CHAR_UUID_HEART_RATE,
    NIMBLE_PROPERTY::NOTIFY
  );
  heartRateChar->setValue((uint8_t*)"\x00", 1);

  NimBLEService *stepService = pServer->createService(SERVICE_UUID_STEP_COUNTER);
  stepCountChar = stepService->createCharacteristic(
    CHAR_UUID_STEP_COUNT,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  stepCountChar->setValue("0");

  batteryService->start();
  heartRateService->start();
  stepService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_BATTERY);
  pAdvertising->addServiceUUID(SERVICE_UUID_HEART_RATE);
  pAdvertising->addServiceUUID(SERVICE_UUID_STEP_COUNTER);
  pAdvertising->start();

  Serial.println("BLE GATT server started.");
  Serial.print("Device Address: ");
  Serial.println(NimBLEDevice::getAddress().toString().c_str());
}

void loop() {
  static int batteryLevel = 100;
  static int heartRate = 70;
  static int steps = 0;

  batteryLevel = max(0, batteryLevel - 1);
  heartRate = random(60, 100);
  steps += random(1, 5);

  batteryLevelChar->setValue(batteryLevel);
  batteryLevelChar->notify();

  uint8_t hrmData[2] = { 0x00, (uint8_t)heartRate };
  heartRateChar->setValue(hrmData, 2);
  heartRateChar->notify();

  stepCountChar->setValue(String(steps).c_str());
  stepCountChar->notify();

  delay(3000);
}