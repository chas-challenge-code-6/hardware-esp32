/**
 * @file battery.cpp
 * @brief Battery Monitoring Implementation
 * 
 */

#include "battery.h"
#include "utilities.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

#ifndef BOARD_BAT_ADC_PIN
#error "No support this board - BOARD_BAT_ADC_PIN not defined"
#endif

// Supported boards:
// - LILYGO T-SIM7670G-S3: ESP32-S3 based with SIM7670G modem
// - LILYGO T-A7670: ESP32 based with A7670 modem
// Battery ADC Pin: Defined in utilities.h for each board


/**
 * @brief Initializes the battery monitoring system
 * 
 */
void Battery::begin()
{
    safePrintln("Initializing battery monitoring...");

#ifdef LILYGO_T_SIM7670G_S3
    Serial.println("Board: LILYGO T-SIM7670G-S3 (ESP32-S3)");
    Serial.printf("Battery ADC Pin: %d\n", BOARD_BAT_ADC_PIN);
#elif defined(LILYGO_T_A7670)
    Serial.println("Board: LILYGO T-A7670 (ESP32)");
    Serial.printf("Battery ADC Pin: %d\n", BOARD_BAT_ADC_PIN);
#endif

    // ADC setting - You don't need to set it, because the values are all default.
    analogSetAttenuation(ADC_11db); // Set the ADC attenuation to 11dB (0-3.6V)
    analogReadResolution(12);       // Set the ADC resolution to 12 bits (0-4095)

    // Configure ADC width based on chip type
#if CONFIG_IDF_TARGET_ESP32
    analogSetWidth(12); // ESP32: Set the ADC width to 12 bits (0-4095)
    Serial.println("ESP32 ADC configured (12-bit with analogSetWidth)");
#else
    // ESP32-S3: analogSetWidth is not available, uses default 12-bit resolution
    safePrintln("ESP32-S3 ADC configured (12-bit default)");
#endif

    // Initialize the battery pin
    pinMode(BOARD_BAT_ADC_PIN, INPUT);

    // Initial battery voltage reading
    uint32_t battery_voltage_mv = getBatteryVoltage();
    safePrintf("Initial battery voltage: %u mV\n", battery_voltage_mv);

    // Check if battery voltage is critically low
    if (battery_voltage_mv < LOW_VOLTAGE_LEVEL)
    {
        safePrintf("Battery voltage is too low, %u mV, entering sleep mode\n",
                      battery_voltage_mv);
        //deepSleep(SLEEP_MINUTE * 60 * 1000); // 60 minutes
    }

    safePrintln("Battery monitoring initialized successfully");
}


/**
 * @brief Gets the average battery voltage over multiple readings
 * 
 * @return uint32_t 
 */
uint32_t Battery::getBatteryVoltageAverage()
{
    std::vector<uint32_t> data;
    for (int i = 0; i < 30; ++i)
    {
        uint32_t val = analogReadMilliVolts(BOARD_BAT_ADC_PIN);
        data.push_back(val);
        delay(30);
    }
    std::sort(data.begin(), data.end());
    data.erase(data.begin());
    data.pop_back();
    int sum = std::accumulate(data.begin(), data.end(), 0);
    double average = static_cast<double>(sum) / data.size();
    return average * 2;
}

/**
 * @brief Gets the current battery voltage in millivolts
 * 
 * @return uint32_t 
 */

uint32_t Battery::getBatteryVoltage()
{
    return getBatteryVoltageAverage();
}

/**
 * @brief Reads the battery voltage in volts
 * 
 * @return float 
 */
float Battery::readVoltage()
{
    uint32_t voltage_mv = getBatteryVoltage();
    return voltage_mv / 1000.0;
}

/**
 * @brief Gets the battery percentage based on the voltage
 * 
 * @return int 
 */
int Battery::percent()
{
    float percentage = getBatteryStatus();
    return static_cast<int>(percentage + 0.5f);
}

/**
 * @brief Gets the battery status as a percentage based on the voltage
 * 
 * @return float 
 */
float Battery::getBatteryStatus()
{
    uint32_t voltage_mv = getBatteryVoltage(); // Get the battery voltage in millivolts
    float voltage = voltage_mv / 1000.0;       // Convert to volts

    // Define min and max voltages for lithium battery (adjust as needed)
    const float minVoltage = 3.3; // 0% (empty)
    const float maxVoltage = 4.2; // 100% (full)

    // Clamp voltage to min/max range
    if (voltage < minVoltage)
        voltage = minVoltage;
    if (voltage > maxVoltage)
        voltage = maxVoltage;

    // Calculate percentage
    float percentage = ((voltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;

    return percentage;
}

/**
 * @brief Gets the battery update every 10 seconds
 * 
 */
void Battery::getUpdate()
{
    static unsigned long lastRead = 0;
    unsigned long now = millis();
    if (now - lastRead >= 10000)
    { // 10,000 ms = 10 seconds
        uint32_t voltage_mv = getBatteryVoltage();
        float percent = getBatteryStatus();
        safePrintf("10s Battery Update: %u mV | %.1f%%\n", voltage_mv, percent);
        lastRead = now;
    }
}

/**
 * @brief Turns on the power pin
 * 
 * @param pin 
 */
void Battery::turnOnPower(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    safePrintf("Power pin %d set to HIGH\n", pin);
}

/**
 * @brief Turns off the power pin
 * 
 * @param pin 
 */
void Battery::turnOffPower(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    safePrintf("Power pin %d set to LOW\n", pin);
}

/**
 * @brief Checks if the power pin is on
 * 
 * @param pin 
 * @return true 
 * @return false 
 */
bool Battery::isPowerOn(int pin)
{
    pinMode(pin, INPUT);
    bool powerState = digitalRead(pin) == HIGH;
    safePrintf("Power pin %d state: %s\n", pin, powerState ? "ON" : "OFF");
    return powerState;
}


/**
 * @brief Initiates a safety shutdown due to low battery voltage
 * 
 */
void Battery::safetyShutdown()
{
    safePrintln("Safety shutdown initiated - Low battery voltage detected");
    safePrintln("Entering deep sleep mode...");
    deepSleep(SLEEP_MINUTE * 60 * 1000);
}

/**
 * @brief Puts the device into deep sleep mode for a specified duration
 * 
 * @param ms Duration in milliseconds
 */
void Battery::deepSleep(uint32_t ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_deep_sleep_start();
}

/**
 * @brief Enters power save mode by putting the device into deep sleep for a short duration
 * 
 */
void Battery::powerSaveMode()
{
    safePrintln("Entering deep sleep mode for 10 seconds...");
    Serial.flush();   // Ensure all serial output is sent
    deepSleep(10000); // 10 seconds
    safePrintln("Woke up from deep sleep!");
}

/**
 * @brief Sets the RGB LED color based on battery percentage
 * 
 * @param percent 
 */
void Battery::setRGB(int percent)
{
    if (percent <= 20) {
        analogWrite(RGB_GREEN_PIN, 0);
        analogWrite(RGB_RED_PIN, 255);
        analogWrite(RGB_BLUE_PIN, 0);
    } else if (percent <= 50) {
        analogWrite(RGB_GREEN_PIN, 255);
        analogWrite(RGB_RED_PIN, 255);
        analogWrite(RGB_BLUE_PIN, 0);
    } else {
        analogWrite(RGB_GREEN_PIN, 255);
        analogWrite(RGB_RED_PIN, 0);    
        analogWrite(RGB_BLUE_PIN, 0);
    }
}