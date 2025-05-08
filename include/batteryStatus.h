#ifndef BATTERYSTATUS_H
#define BATTERYSTATUS_H
#include "utilities.h"
#include <Arduino.h>
#include <esp32-hal-adc.h> /// Include the ESP32 ADC library for analogReadMilliVolts

class BatteryStatus 
{
    private:
        int batteryPin;
        float batteryVoltage;
        int batteryPercentage;
        int voltageDivider;

    public:
        BatteryStatus(); // Constructor
        void begin(); // Set console baud rate
        void loop();
        float getBatteryStatus();
        void printBatteryStatus();
        float getBatteryVoltage();
        void powerSaveMode();
};

#endif //BATTERYSTATUS_H