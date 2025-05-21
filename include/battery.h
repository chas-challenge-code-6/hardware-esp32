#ifndef BATTERY_H
#define BATTERY_H
#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>


class BatteryMonitor {
public:
    BatteryMonitor(int adcPin, float vMax = 4.2, float vMin = 3.3, float divider = 2.0);

    // Returns battery voltage in volts
    float readVoltage();

    // Returns battery percent (0-100)
    int percent();

private:
    int _adcPin;
    float _vMax, _vMin, _divider;
};

/* #define SerialAT Serial1
extern TinyGsm modem; // Declare as extern, define in battery.cpp */

/* class Battery 
{
    private: 
        int modem_rx; // RX pin for modem
        int modem_tx; // TX pin for modem

    public:
        Battery(int rx = 32, int tx = 31) : modem_rx(rx), modem_tx(tx) {};
        void begin();
        void loop();
        float getBatteryStatus(); // Returns battery percentage
        float getBatteryVoltage(); // Returns battery voltage in mV
        void getUpdate();
        void sendData();

        void powerSaveMode();
        bool isPowerOn(int pin);
        void turnOnPower(int pin);
        void turnOffPower(int pin);
        void safetyShutdown(int pin, float VoltLimit); // pin should be int
}; */

#endif //BATTERY_H