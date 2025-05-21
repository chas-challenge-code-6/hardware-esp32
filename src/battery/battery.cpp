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

/* #include "battery.h"
#include "main.h" // Ensure BATTERY_PINRX is defined

TinyGsm modem(SerialAT); // Define modem here

void Battery::begin()
{
    Serial.begin(115200); 
    SerialAT.begin(115200, SERIAL_8N1, modem_rx, modem_tx); // Use class members
    Serial.println("Starting modem...");
    modem.restart(); 
}

void Battery::loop()
{
    // Example: Periodically print battery info and manage power
    turnOnPower(4); // Example pin
    getBatteryVoltage();
    getBatteryStatus();
    delay(1000);
    safetyShutdown(4, 3.8); // Example pin and voltage limit
    sendData();
    powerSaveMode();
    getUpdate();
    delay(1000);
}

float Battery::getBatteryVoltage()
{
    // Read battery voltage from analog pin (BATTERY_PINRX defined in main.h)
    int raw = analogRead(BATTERY_PINRX);
    // Example calculation for 12-bit ADC (0-4095), 3.3V ref, voltage divider ratio 2:1
    float voltage = (raw / 4095.0) * 3.3 * 2.0; // Adjust multiplier for your hardware
    Serial.print("Battery Voltage: ");
    Serial.print(voltage);
    Serial.println(" V");
    return voltage;
}

float Battery::getBatteryStatus()
{
    float voltage = getBatteryVoltage();
    // Map voltage to percentage (example: 3.0V = 0%, 4.2V = 100%)
    float percent = (voltage - 3.0) / (4.2 - 3.0) * 100.0;
    percent = constrain(percent, 0, 100);
    Serial.print("Battery Percentage: ");
    Serial.print(percent);
    Serial.println(" %");
    return percent;
}

// Not done yet
void Battery::getUpdate()
{
    static unsigned long lastRead = 0;
    unsigned long now = millis();
    if (now - lastRead >= 10000)
    { // 10,000 ms = 10 seconds
        float voltage = getBatteryVoltage();
        float percent = getBatteryStatus();
        Serial.print("10s Battery Voltage: ");
        Serial.print(voltage);
        Serial.print(" | Battery %: ");
        Serial.println(percent);
        lastRead = now;
    }
}

// Functions For power managment
void Battery::turnOnPower(int pin)
{
    // Turn on the power
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    if (isPowerOn(pin)) 
    {
        Serial.println("Power is ON");
    }
    else
    {
        Serial.println("Power is OFF");
    }
}

void Battery::turnOffPower(int pin)
{
    // Turn off the power
    pinMode(pin, INPUT);
    digitalWrite(pin, LOW);
    isPowerOn(pin);
}

// Checks if the pin is recieving power
bool Battery::isPowerOn(int pin)
{
    // Check if the pin is receiving power
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
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
        return false;
    }
}


void Battery::safetyShutdown(int pin, float VoltLimit)
{
    float voltage = getBatteryVoltage();
    if (voltage < VoltLimit)
    {
        Serial.println("Safety shutdown: Everything is okay!");
    }
    else if (voltage > VoltLimit)
    {
        Serial.println("Warning! Battery voltage is above the limit");
        Serial.println("Initiating safety shutdown process...");
        turnOffPower(pin);
        isPowerOn(pin);
        Serial.println("Power is OFF");
    }
}

void Battery::sendData() {
    // only send data when connected
    //     if (connected) {
    //         if (millis() - timeStamp > 1000) {
    //             timeStamp = millis();

    // #else
    //             snprintf(buf, 256, "Battery:%umV ", battery_voltage);
    // #endif

    //             // When connected to the USB, the battery voltage data read is not the real
    //             battery voltage,
    //             // so the battery voltage is sent to the UDP Server through UDP. When using it,
    //             please disconnect the USBC
    //             Serial.println(buf);
};
 */