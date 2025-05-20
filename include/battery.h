#ifndef BATTERY_H
#define BATTERY_H
#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>

#define SerialAT Serial1
extern TinyGsm modem; // Declare as extern, define in battery.cpp

class Battery 
{
    private: 
        

    public:
        Battery(int rx = 32, int tx = 31) : modem_rx(rx), modem_tx(tx) {}
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
};

#endif //BATTERY_H