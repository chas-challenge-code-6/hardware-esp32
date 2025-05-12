#ifndef BATTERY_H
#define BATTERY_H
#include "utilities.h"
#include <Arduino.h>
#include <esp32-hal-adc.h> /// Include the ESP32 ADC library for analogReadMilliVolts

class Battery 
{
    private:
        int batteryPin;
        float batteryVoltage;
        float rawBatteryVoltage;
        float batteryPercentage;
        int voltageDivider;
        bool isPowerOn;

    public:
        Battery(); // Constructor
        float getBatteryStatus();
        float getBatteryVoltage();
        void getUpdate();
        void sendUpdate(); //For sending battery status to the server
        void powerSaveMode();
};

#endif //BATTERY_H