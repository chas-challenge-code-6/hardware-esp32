#include "batteryStatus.h"

// Define the global instance
BatteryStatus batteryStatus;

// Constructor
BatteryStatus::BatteryStatus() {
  batteryVoltage = 0.0;
  status = 0;
  lastReadTime = 0;
};

// Initialize battery monitoring
void BatteryStatus::begin() {
  // Configure ADC
  analogReadResolution(12); // ESP32 has 12-bit ADC resolution
  
  // Set up the ADC pin as input
  pinMode(BATTERY_ADC_PIN, INPUT);
  
  // Take initial reading
  update();
}

// Update battery readings
void BatteryStatus::update() {
  unsigned long currentTime = millis();
  
  // Only update if the interval has passed
  if (currentTime - lastReadTime >= READ_INTERVAL || lastReadTime == 0) {
    batteryVoltage = getBatteryVoltage();
    lastReadTime = currentTime;
    
    // Update battery status based on voltage
    if (batteryVoltage <= BATTERY_CRITICAL_THRESHOLD) {
      status = BATTERY_CRITICAL;
    } else if (batteryVoltage <= BATTERY_LOW_THRESHOLD) {
      status = BATTERY_LOW;
    } else {
      status = BATTERY_NORMAL;
    }
  }
}

// Get current battery voltage
float BatteryStatus::getBatteryVoltage() {
  return batteryVoltage;
}

// Get current battery status
BatteryStatus_t BatteryStatus::getStatus() {
  return getStatus;
}

// Get battery percentage (0-100%)
int BatteryStatus::getBatteryPercentage() {
  // Calculate percentage based on voltage range
  float percentage = (batteryVoltage - BATTERY_CRITICAL_THRESHOLD) / 
                    (BATTERY_MAX_VOLTAGE - BATTERY_CRITICAL_THRESHOLD) * 100.0;
  
  // Constrain to 0-100 range
  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;
  
  return (int)percentage;
}

// Check if device should enter power saving mode
bool BatteryStatus::isPowerSaving() {
  // Return true if battery is low or critical
  return (status == BATTERY_LOW || status == BATTERY_CRITICAL);
}

// Control power to the board
void BatteryStatus::enablePower(bool enable) {
  // This function would control a power management pin or circuit
  // Implementation depends on your specific hardware setup
  // For example:
  // digitalWrite(POWER_CONTROL_PIN, enable ? HIGH : LOW);
}

// Read raw battery voltage from ADC
float BatteryStatus::readBatteryVoltage() {
  // Take multiple readings for stability
  const int numReadings = 10;
  int total = 0;
  
  for (int i = 0; i < numReadings; i++) {
    total += analogRead(BATTERY_ADC_PIN);
    delay(5);
  }
  
  int averageReading = total / numReadings;
  
  // Convert ADC reading to voltage
  // Assuming a voltage divider is used to bring battery voltage into ADC range
  // You'll need to adjust these values based on your specific circuit
  float adcVoltage = (averageReading / 4095.0) * 3.3; // ESP32 ADC reference is 3.3V
  
  // If using a voltage divider with R1 and R2, calculate the actual battery voltage
  // For example, with R1=100k and R2=100k (1:1 divider):
  float batteryVoltage = adcVoltage * 2.0; // Adjust this multiplier based on your voltage divider
  
  return batteryVoltage;
}


//  /* @note      Reading the battery voltage information is only applicable to the V1.2 version.
//  *            T-A7670x :  The V1.1 version does not have a battery voltage divider.
//  *                         If V1.1 needs to be read, then you need to add a 100K 1% voltage divider resistor between the battery and GND
//  *                         When connected to the USB, the battery voltage data read is not the real battery voltage, so the battery
//  *                         voltage is sent to the UDP Server through UDP. When using it, please disconnect the USB-C
//  *            T-A7670x :  Only version V1.4 has the resistor divider connected to the solar input, other versions IO38 is not connected
//  * @note      Only support T-A7670 ,T-A7608X, T-SIM7672G board , not support T-Call A7670 , T-PCIE-A7670
//  */
// #include "include\batteryStatus.h"
// #include <esp32-hal-adc.h>
// #include "C:\chas\SUVX24\Projects\Chas_challenge_2025\Team-Code_6_Project_Sentinel\hardware-esp32\include\utilities.h"


// #ifndef BOARD_BAT_ADC_PIN
// #error "No support this board"
// #endif


// void setup()
// {
//     Serial.begin(115200); // Set console baud rate

// #ifdef BOARD_POWERON_PIN
//     pinMode(BOARD_POWERON_PIN, OUTPUT);
//     digitalWrite(BOARD_POWERON_PIN, HIGH);
// #endif


//     //adc setting start
     
//     // You don't need to set it, because the values ​​are all default. The current version is Arduino 3.0.4, and the subsequent versions are uncertain.
    
//     analogSetAttenuation(ADC_11db);

//     analogReadResolution(12);

// #if CONFIG_IDF_TARGET_ESP32
//     analogSetWidth(12);
// #endif

//     //adc setting end 
// }

// void loop()
// {
// //     //only send data when connected
// //     if (connected) {
// //         if (millis() - timeStamp > 1000) {
// //             timeStamp = millis();

// //             uint32_t battery_voltage = analogReadMilliVolts(BOARD_BAT_ADC_PIN);
// //             battery_voltage *= 2;   //The hardware voltage divider resistor is half of the actual voltage, multiply it by 2 to get the true voltage


// // #else
// //             snprintf(buf, 256, "Battery:%umV ", battery_voltage);
// // #endif


// //             // When connected to the USB, the battery voltage data read is not the real battery voltage,
// //             // so the battery voltage is sent to the UDP Server through UDP. When using it, please disconnect the USBC
// //             Serial.println(buf);
// //         }
// //     }
// }