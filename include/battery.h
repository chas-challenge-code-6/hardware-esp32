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

#include "config.h"
#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>
#include <algorithm>
#include <esp32-hal-adc.h>
#include <numeric>
#include <vector>

// Voltage thresholds
#define LOW_VOLTAGE_LEVEL 3600
#define WARN_VOLTAGE_LEVEL 3700
#define SLEEP_MINUTE 60

class Battery
{
private:
    static uint32_t getBatteryVoltageAverage();

public:
    void begin();
    float getBatteryStatus();
    int percent();
    uint32_t getBatteryVoltage();
    float readVoltage();
    void getUpdate();

    void powerSaveMode();
    void deepSleep(uint32_t ms);
    bool isPowerOn(int pin);
    void turnOnPower(int pin);
    void turnOffPower(int pin);
    void safetyShutdown();
};

#endif
