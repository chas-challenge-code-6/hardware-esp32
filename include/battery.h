#ifndef BATTERY_H
#define BATTERY_H

#include "utilities.h"
#include <Arduino.h>
#include <esp32-hal-adc.h>

class Battery
{
private:
    int batteryPin;
    float ADCbatteryVoltage;
    float rawBatteryVoltage;
    float batteryVoltageLimit;
    float batteryPercentage;
    int voltageDivider;

public:
    // Functions for monitoring battery status
    void begin();
    void loop();
    float getBatteryStatus();
    float getBatteryVoltage();
    void getUpdate();
    void sendData(); // For sending battery status to the server

    // Functions for power managment
    void powerSaveMode();
    bool isPowerOn(int pin);
    void turnOnPower(int pin);
    void turnOffPower(int pin);
    void safetyShutdown(float pin, float VoltLimit);
};

#endif