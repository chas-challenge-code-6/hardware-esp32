#include "batteryStatus.h"
#include <esp32-hal-adc.h> // Include the ESP32 ADC library for analogReadMilliVolts
#include "utilities.h" // Include the utilities header for utility functions

//Constructor for BatteryStatus class
BatteryStatus::BatteryStatus()
{
};

// Initialize the battery status
void BatteryStatus::begin()
{ 
};



// The documentation for easier implementation


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