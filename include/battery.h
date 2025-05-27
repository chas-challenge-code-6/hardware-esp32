/**
 * @file battery.h
 * @brief Battery monitoring system for LILYGO ESP32-based boards
 * 
 * Supports:
 * - LILYGO T-SIM7670G-S3 (ESP32-S3 based)
 * - LILYGO T-A7670 (ESP32 based)
 * 
 * Features:
 * - High-accuracy voltage readings with 30-sample averaging
 * - Outlier removal for stable measurements
 * - Safety voltage thresholds and automatic shutdown
 * - Power management with light/deep sleep modes
 * - Battery percentage calculation for lithium batteries
 */

#ifndef BATTERY_H
#define BATTERY_H

#include "utilities.h"
#include "config.h"
#include <Arduino.h>
#include <TinyGsmClient.h>
#include <esp32-hal-adc.h>
#include <vector>
#include <algorithm>
#include <numeric>

// Voltage thresholds
#define LOW_VOLTAGE_LEVEL       3600            // Sleep shutdown voltage (mV)
#define WARN_VOLTAGE_LEVEL      3700            // Warning voltage (mV)
#define SLEEP_MINUTE            60              // Sleep time, unit / minute

class Battery
{
private:
    static uint32_t getBatteryVoltageAverage(); // Static method for accurate voltage reading

public:
    // Functions for monitoring battery status
    void begin();
    float getBatteryStatus(); // Returns percentage as float
    int percent(); // Returns percentage as int (for batteryTask compatibility)
    uint32_t getBatteryVoltage(); // Returns voltage in mV
    float readVoltage(); // Returns voltage in volts (for batteryTask compatibility)
    void getUpdate();
    
    // Functions for power management
    void powerSaveMode();
    void deepSleep(uint32_t ms);
    bool isPowerOn(int pin);
    void turnOnPower(int pin);
    void turnOffPower(int pin);
    void safetyShutdown(); // Fixed to use actual voltage reading
    void setRGB(int percent);
};

#endif
