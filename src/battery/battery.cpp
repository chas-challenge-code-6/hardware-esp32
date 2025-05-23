#include "battery.h"

BatteryMonitor::BatteryMonitor(int adcPin, float vMax, float vMin, float divider)
    : _adcPin(adcPin), _vMax(vMax), _vMin(vMin), _divider(divider) {}

float BatteryMonitor::readVoltage() {
    uint32_t mv = analogReadMilliVolts(_adcPin);
    return (mv * _divider) / 1000.0;
}

int BatteryMonitor::percent() {
    float v = readVoltage();
    if (v <= _vMin) return 0;
    if (v >= _vMax) return 100;
    return int(100 * (v - _vMin) / (_vMax - _vMin) + 0.5f);
}