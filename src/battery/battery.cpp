#include "battery.h"

Battery battery; // Create an instance of the Battery class

//Constructor for Battery class
Battery::Battery()
{
    this->batteryPin = 0;
    this->rawBatteryVoltage = 0;
    this->batteryVoltage = 0;
    this->batteryPercentage = 0;
    this->voltageDivider = 0;
    this->isPowerOn = false;

    Serial.begin(115200); // Set console baud rate
    Serial.println("initializing battery...");
    
    #define BOARD_LED_PIN (12)
    int boardLedPin = BOARD_LED_PIN;

    // Initialize the power pin
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
    if (digitalRead(BOARD_POWERON_PIN) == HIGH)
        {
            isPowerOn = true;
            pinMode(boardLedPin, OUTPUT);
            digitalWrite(boardLedPin, HIGH);
            Serial.println("Power is ON");
        }
    else
        {
            isPowerOn = false;
            pinMode(boardLedPin, OUTPUT);
            digitalWrite(boardLedPin, LOW);
            Serial.println("Power is OFF");
        }

    // Initialize the battery pin
    batteryPin = BOARD_BAT_ADC_PIN;
    pinMode(batteryPin, INPUT);
    rawBatteryVoltage=(analogReadMilliVolts(batteryPin));
    Serial.print("Battery Pin: " + String(batteryPin));
    Serial.print("Raw Battery Voltage: " + String(rawBatteryVoltage)+" mV");
    
    //adc setting start
    // You don't need to set it, because the values ​​are all default. 

    analogSetAttenuation(ADC_11db); // Set the ADC attenuation to 11dB (0-3.6V)
    analogReadResolution(12); // Set the ADC resolution to 12 bits (0-4095)
    #if CONFIG_IDF_TARGET_ESP32
    analogSetWidth(12); // Set the ADC width to 12 bits (0-4095)
    #endif

    voltageDivider = 100000; // Set the voltage divider to 100K (100K/100K)
    Serial.print("Voltage Divider: " + String(voltageDivider));
    Serial.print("ADC Attenuation: " + String(ADC_11db));
    Serial.print("ADC Resolution: " + String(12));
    Serial.print("Battery sucessfully initialized");
};

float Battery::getBatteryVoltage()
{
    // Read the battery voltage
    rawBatteryVoltage = analogReadMilliVolts(batteryPin);
    batteryVoltage = (rawBatteryVoltage / voltageDivider) * 2; // Calculate the battery voltage using the voltage divider
    return batteryVoltage;  // The hardware voltage divider resistor is half of the actual voltage, multiply it by 2 to get the true voltage

};
// Everything up to this point is fine

float Battery::getBatteryStatus()
{
    float voltage = getBatteryVoltage();
    //float batteryLevel = (voltage / voltageDivider *=2); // Calculate the battery level as a percentage
   // this->batteryPercentage = batteryLevel;
    //return batteryLevel;
};


void Battery::getUpdate()
{

};

//For sending battery status to the smart watch or server
void sendUpdate()
{
};
 
void powerSaveMode()
{};

// The documentation for easier implementation

//  /* @note      Reading the battery voltage information is only applicable to the V1.2 version.
//  *            T-A7670x :  The V1.1 version does not have a battery voltage divider.
//  *                         If V1.1 needs to be read, then you need to add a 100K 1% voltage divider resistor between the battery and GND
//  *                         When connected to the USB, the battery voltage data read is not the real battery voltage, so the battery
//  *                         voltage is sent to the UDP Server through UDP. When using it, please disconnect the USB-C
//  *            T-A7670x :  Only version V1.4 has the resistor divider connected to the solar input, other versions IO38 is not connected
//  * @note      Only support T-A7670 ,T-A7608X, T-SIM7672G board , not support T-Call A7670 , T-PCIE-A7670


// void loop()
// {
// //     //only send data when connected
// //     if (connected) {
// //         if (millis() - timeStamp > 1000) {
// //             timeStamp = millis();

// // #else
// //             snprintf(buf, 256, "Battery:%umV ", battery_voltage);
// // #endif


// //             // When connected to the USB, the battery voltage data read is not the real battery voltage,
// //             // so the battery voltage is sent to the UDP Server through UDP. When using it, please disconnect the USBC
// //             Serial.println(buf);
// //         }
// //     }
// }