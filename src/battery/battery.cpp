#include "battery.h"

// The documentation for our specific board (If needed): LILYGO T-SIM7000G S3

// #elif defined(LILYGO_T_SIM7670G_S3)
// #define MODEM_BAUDRATE (115200)
// #define MODEM_DTR_PIN (9)
// #define MODEM_TX_PIN (11)
// #define MODEM_RX_PIN (10)

// // The modem boot pin needs to follow the startup sequence.
// #define BOARD_PWRKEY_PIN (18)
// #define BOARD_LED_PIN (12)

// // There is no modem power control, the LED Pin is used as a power indicator here.
// #define BOARD_POWERON_PIN (BOARD_LED_PIN)
// #define MODEM_RING_PIN (3)
// #define MODEM_RESET_PIN (17)
// #define MODEM_RESET_LEVEL LOW
// #define SerialAT Serial1

// #define BOARD_BAT_ADC_PIN (4)
// #define BOARD_SOLAR_ADC_PIN (5)
// #define BOARD_MISO_PIN (47)
// #define BOARD_MOSI_PIN (14)
// #define BOARD_SCK_PIN (21)
// #define BOARD_SD_CS_PIN (13)

// #ifndef TINY_GSM_MODEM_SIM7672
// #define TINY_GSM_MODEM_SIM7672
// #endif

// #define MODEM_GPS_ENABLE_GPIO (4)
// #define MODEM_GPS_ENABLE_LEVEL (1)


void Battery::begin()
{
    Serial.begin(115200); // Set console baud rate
    Serial.println("initializing battery...");
    #define BOARD_BAT_ADC_PIN (4)  //Battery pin
    #define BOARD_POWERON_PIN (BOARD_LED_PIN) // There is no modem power control, the LED Pin is used as a power indicator here.
    #define BOARD_LED_PIN (12)
    
    //Variables
    int batteryPin=BOARD_BAT_ADC_PIN;
    int boardLedPin = BOARD_LED_PIN;
    float rawBatteryVoltage=(analogReadMilliVolts(batteryPin)); //Reads the raw battery voltage
    float ADCbatteryVoltage= rawBatteryVoltage/voltageDivider*2; //Analog/Digital Converter battery voltage
    float batteryPercentage;
    int voltageDivider=10000; // Set the voltage divider to 100K (100K/100K)
    bool isPowerOn=false;
    float batteryVoltageLimit=3.8; // Set the battery voltage limit to 3.8V
    
    //adc setting start
    // You don't need to set it, because the values ​​are all default. 
    analogSetAttenuation(ADC_11db); // Set the ADC attenuation to 11dB (0-3.6V)
    analogReadResolution(12); // Set the ADC resolution to 12 bits (0-4095)
    #if CONFIG_IDF_TARGET_ESP32 
    analogSetWidth(12); // Set the ADC width to 12 bits (0-4095)
    #endif

    // Initialize the battery pin
    pinMode(batteryPin, INPUT);
    analogReadMilliVolts(batteryPin); // Read the battery voltage
    turnOnPower(batteryPin); // Turn on the power
    pinMode(boardLedPin, OUTPUT);

    // Print the battery status
    Serial1.print("Battery Pin: " + String(batteryPin));
    Serial1.print("Raw Battery Voltage: " + String(rawBatteryVoltage)+" mV");
    Serial.print("ADC Battery Voltage: " + String(ADCbatteryVoltage)+" mV");
    Serial.print("Voltage Divider: " + String(voltageDivider));
    Serial.print("ADC Attenuation: " + String(ADC_11db));
    Serial.print("ADC Resolution: " + String(12));
    Serial.print("Battery sucessfully initialized");
};



void Battery::loop()
{   
    // Call the loop method of the Battery class
    // Battery battery(batteryPin);
    //  Battery::isPowerOn(batteryPin); // Check if the power is on or off

    Battery::turnOnPower(batteryPin);
    isPowerOn(batteryPin);
    Battery::getBatteryVoltage();
    Serial.print("Battery Voltage: " + String(ADCbatteryVoltage)+" mV");

    Battery::getBatteryStatus();
    delay(1000); // Delay for 1 second

    Battery::safetyShutdown(batteryPin, batteryVoltageLimit); // Call the safetyShutdown method
    Battery::sendData(); // Call the sendData method
    Battery::powerSaveMode(); // Call the powerSaveMode method
    Battery::getUpdate(); // Call the getUpdate method of the Battery class
    delay(1000); // Delay for 1 second
}
    

//Functions for monitoring battery status
    float Battery::getBatteryVoltage()
    {
        analogReadMilliVolts(batteryPin); // Read the battery voltage
        rawBatteryVoltage=(analogReadMilliVolts(batteryPin)); //Reads the raw battery voltage
        ADCbatteryVoltage= rawBatteryVoltage/voltageDivider*2; //Analog/Digital Converter battery voltage
        // Get the battery voltage
        float rawBatteryVoltage=(analogReadMilliVolts(batteryPin)); //Reads the raw battery voltage
        float ADCbatteryVoltage= rawBatteryVoltage/voltageDivider*2; //Analog/Digital Converter battery voltage
        Serial.println("Reading Battery Voltage...");
        Serial.println("Raw voltage: " +String(rawBatteryVoltage)+" mV");
        Serial.println("Voltage divider (100k): " + String(voltageDivider)+" mV");
        Serial.println("Divided with the voltage divider and multiplied by 2");
        Serial.println("The battery voltage is: " + String(ADCbatteryVoltage)+" mV");

        return ADCbatteryVoltage;
    };


    //Not done yet
    float Battery::getBatteryStatus()
    {
        getBatteryVoltage(); // Call the getBatteryVoltage method


    };
    //Not done yet
    void Battery::getUpdate()
    {};
    
    // Functions For power managment
    void Battery::turnOnPower(int pin)
    {
        // Turn on the power
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        if (isPowerOn(pin)==true)
        {
            Serial.println("Power is ON");
        }
        else
        {
            Serial.println("Power is OFF");
        }
    };

    void Battery::turnOffPower(int pin)
    {
        // Turn off the power
        pinMode(pin, INPUT);
        analogWrite(pin, LOW);
        isPowerOn(pin);
    };

    //Checks if the pin is recieving power
    float Battery::isPowerOn(int pin)
    {
        // Check if the pin is receiving power
        pinMode(pin, INPUT);
        digitalWrite(pin, HIGH);
        isPowerOn(pin);
        if (digitalRead(pin) == HIGH)
        {
            Serial.println("Power is ON");
            return true;
        }
        else if (digitalRead(pin) == LOW)
        {
            Serial.println("Power is OFF");
            return false;
        }
        else
        {
            Serial.println("Something went wrong");
        }
    }

    void Battery::powerSaveMode()
    {
        // Set the power save mode
       
    };

    //Safety shutdown
    void Battery::safetyShutdown(float pin, float VoltLimit)
    {
        // Check if the battery voltage is below the limit
        if (pin < VoltLimit)
        {
            Serial.println("Safetry shutdown: Everything is okay!");
        }
        else if (pin > VoltLimit)
        {
            Serial.println("Warning! Battery voltage is above the limit");
            Serial.println("Initiating safety shutdown process...");
            turnOffPower(pin);
            isPowerOn(pin);
            Serial.println("Power is OFF");
        }
    };
    //For sending battery status to the smart watch or server
    void Battery::sendData()
    { 
        //only send data when connected
    //     if (connected) {
    //         if (millis() - timeStamp > 1000) {
    //             timeStamp = millis();

    // #else
    //             snprintf(buf, 256, "Battery:%umV ", battery_voltage);
    // #endif


    //             // When connected to the USB, the battery voltage data read is not the real battery voltage,
    //             // so the battery voltage is sent to the UDP Server through UDP. When using it, please disconnect the USBC
    //             Serial.println(buf);
    };
