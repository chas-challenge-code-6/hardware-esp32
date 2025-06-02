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

    if (battery_voltage_mv < LOW_VOLTAGE_LEVEL)
    {
        safePrintf("Battery voltage is too low, %u mV, entering sleep mode\n",
                      battery_voltage_mv);
        //deepSleep(SLEEP_MINUTE * 60 * 1000); // 60 minutes
    }

    safePrintln("Battery monitoring initialized successfully");
}

uint32_t Battery::getBatteryVoltageAverage()
{
    std::vector<uint32_t> data;
    for (int i = 0; i < 30; ++i)
    {
        uint32_t val = analogReadMilliVolts(BOARD_BAT_ADC_PIN);
        data.push_back(val);
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    std::sort(data.begin(), data.end());
    data.erase(data.begin());
    data.pop_back();
    int sum = std::accumulate(data.begin(), data.end(), 0);
    double average = static_cast<double>(sum) / data.size();
    return average * 2;
}

uint32_t Battery::getBatteryVoltage()
{
    return getBatteryVoltageAverage();
}

float Battery::readVoltage()
{
    uint32_t voltage_mv = getBatteryVoltage();
    return voltage_mv / 1000.0;
}

int Battery::percent()
{
    float percentage = getBatteryStatus();
    return static_cast<int>(percentage + 0.5f);
}

float Battery::getBatteryStatus()
{
    uint32_t voltage_mv = getBatteryVoltage(); // Get the battery voltage in millivolts
    float voltage = voltage_mv / 1000.0;       // Convert to volts

    // Define min and max voltages for lithium battery
    const float minVoltage = 3.3; // 0% (empty)
    const float maxVoltage = 4.2; // 100% (full)

    // Clamp voltage to min/max range
    if (voltage < minVoltage)
        voltage = minVoltage;
    if (voltage > maxVoltage)
        voltage = maxVoltage;

    float percentage = ((voltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;

    return percentage;
}

void Battery::getUpdate()
{
    static unsigned long lastRead = 0;
    unsigned long now = millis();
    if (now - lastRead >= 10000)
    {
        uint32_t voltage_mv = getBatteryVoltage();
        float percent = getBatteryStatus();
        safePrintf("10s Battery Update: %u mV | %.1f%%\n", voltage_mv, percent);
        lastRead = now;
    }
}

void Battery::turnOnPower(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    safePrintf("Power pin %d set to HIGH\n", pin);
}

void Battery::turnOffPower(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    safePrintf("Power pin %d set to LOW\n", pin);
}

bool Battery::isPowerOn(int pin)
{
    pinMode(pin, INPUT);
    bool powerState = digitalRead(pin) == HIGH;
    safePrintf("Power pin %d state: %s\n", pin, powerState ? "ON" : "OFF");
    return powerState;
}

void Battery::safetyShutdown()
{
    safePrintln("Safety shutdown initiated - Low battery voltage detected");
    safePrintln("Entering deep sleep mode...");
    deepSleep(SLEEP_MINUTE * 60 * 1000);
}

void Battery::deepSleep(uint32_t ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_deep_sleep_start();
}

void Battery::powerSaveMode()
{
    safePrintln("Entering deep sleep mode for 10 seconds...");
    Serial.flush();
    deepSleep(10000);
    safePrintln("Woke up from deep sleep!");
}

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