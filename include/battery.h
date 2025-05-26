#ifndef BATTERY_H
#define BATTERY_H

#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>
#include <esp32-hal-adc.h> /// Include the ESP32 ADC library for analogReadMilliVolts

//LiPo 18650 battery

class BatteryMonitor
{
    private:
        int _adcPin;
        float _vMax, _vMin, _divider;

    public:
        BatteryMonitor(int adcPin, float vMax = 4.2, float vMin = 3.3, float divider = 2.0);
        float readVoltage();
        int percent();
        void sendData(){};
};

class PowerManagement
{
    public:
    PowerManagement();
    bool isPowerOn();
    void powerSaveMode();
    void safetyShutdown(float pin, float vMax);
};


// Original battery class
//
// class Battery
// {
// private:
//     int batteryPin;
//     float ADCbatteryVoltage;
//     float rawBatteryVoltage;
//     float batteryVoltageLimit;
//     float batteryPercentage;
//     int voltageDivider;

// public:
//     // // Functions for monitoring battery status
//     // void begin();
//     // void loop();
//     float getBatteryStatus();
//     float getBatteryVoltage();
//     void getUpdate();
//     void sendData(); // For sending battery status to the server

//     // Functions for power managment
//     void powerSaveMode();
//     bool isPowerOn(int pin);
//     void turnOnPower(int pin);
//     void turnOffPower(int pin);
//     void safetyShutdown(float pin, float VoltLimit);
// };

#endif
