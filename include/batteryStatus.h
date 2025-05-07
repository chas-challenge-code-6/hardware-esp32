#ifndef BATTERYSTATUS_H
#define BATTERYSTATUS_H
#include "main.h"
#include "utilities.h"
#include <Arduino.h>

// Battery status constants
#define BATTERY_LOW_THRESHOLD 3.3      // Voltage threshold for low battery (in volts)
#define BATTERY_CRITICAL_THRESHOLD 3.0  // Voltage threshold for critical battery (in volts)
#define BATTERY_MAX_VOLTAGE 4.2        // Maximum battery voltage (fully charged)
#define BATTERY_ADC_PIN 35             // ADC pin connected to battery voltage divider
#define BATTERY_VOLTAGE_DIVIDER 2      // Voltage divider ratio (e.g., 2 for 1:2 divider)
#define BATTERY_VOLTAGE_RESOLUTION 4096 // ADC resolution (12-bit ADC = 4096 levels)

// Battery status enum
enum BatteryStatus_t {
  BATTERY_CRITICAL, BATTERY_LOW, BATTERY_NORMAL     
};

class BatteryStatus 
{
    private:
        float batteryVoltage; // Battery voltage in volts
        BatteryStatus_t getStatus; // Current battery status
        unsigned long lastReadTime; // Last time battery voltage was read
        const unsigned long READ_INTERVAL =60000; // Read interval in milliseconds
        // const int BATTERY_ADC_PIN = 35; // ADC pin for battery voltage measurement
        // const int BATTERY_VOLTAGE_DIVIDER = 2; // Voltage divider ratio (e.g., 2 for 1:2 divider)

    public:
    // Constructor
        BatteryStatus(); // Constructor
        ~BatteryStatus() {} // Destructor
        };

    //Methods
        void begin(); // Initialize the battery status
        void update(); // Update the battery readiing
        float getBatteryVoltage(); // Get the current battery voltage
        BatteryStatus_t getStatus(); // Get the current battery status
        int getBatteryPercentage(); // Get the battery percentage (0-100%)
        bool isPowerSaving(); // Check if power saving mode is enabled
        void setPowerSavingMode(bool enable); // Enable or disable power saving mode
        void enablePower(bool enable); // Control power to the board
}

extern batteryStatus; // Global instance declaration

#endif // BATTERYSTATUS_H