#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

// DEVICE_ID for API
#define DEVICE_ID "SENTINEL-001";

// RGB macros
#define RGB_RED_PIN 16
#define RGB_GREEN_PIN 15
#define RGB_BLUE_PIN 7

// DHT22 macros
#define DHTTYPE DHT22
#define DHT_PIN 32;

// Macros for Polar H9
#define STRAP_NAME "POLAR H9 EC351E2B"
#define HEARTRATE_SERVICE_UUID "180D"
#define HEARTRATE_CHAR_UUID "2A37"

// Gas sensor macros
#define GAS_PIN (34)
// #define GAS_BOARD               ("ESP-32")
// #define GAS_TYPE                ("MQ-2")
// #define GAS_VOLTAGE_RESOLUTION  (12)
// #define GAS_ADC_BIT_RESOLUTION  (12)
// #define GAS_RATIO_CLEANAIR      (9.83)

// Setup I2C macros
#define SDA_PIN 21
#define SCL_PIN 22

#endif
