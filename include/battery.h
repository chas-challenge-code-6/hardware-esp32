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

#endif //BATTERY_H