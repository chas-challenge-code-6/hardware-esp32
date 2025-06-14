#ifndef CONFIG_H
#define CONFIG_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define DEBUG 0

// fallback to secrets.h.default
#ifdef __has_include
#if __has_include("secrets.h")
#include "secrets.h"
#define USING_SECRETS_H
#elif __has_include("secrets.h.default")
#include "secrets.h.default"
#warning                                                                                           \
    "secrets.h not found, using secrets.h.default. Please copy secrets.h.default to secrets.h and configure your settings."
#define USING_SECRETS_DEFAULT
#else
#error "Neither secrets.h nor secrets.h.default found. Please ensure secrets.h.default exists."
#endif
#else
// Fallback for compilers that don't support __has_include
#include "secrets.h.default"
#warning "Compiler doesn't support __has_include, using secrets.h.default as fallback."
#define USING_SECRETS_DEFAULT
#endif

// DEVICE_ID for API
#define DEVICE_ID "SENTINEL-001"

// JWT
// #define USE_JWT_AUTH // create token in code
#define USE_BACKEND_AUTH

// Authentication settings
#define AUTH_TIMEOUT_MS 15000 // backend kan ibland ta upp till 50 sek men catchas på retry
#define AUTH_RETRY_ATTEMPTS 3
#define TOKEN_REFRESH_MARGIN_MS 300000  // 5 minuter
#define DEFAULT_TOKEN_EXPIRY_MS 3600000 // 1 timme

// Mutex declarations
extern SemaphoreHandle_t serialMutex;
extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t networkEventMutex;

// Modem settings (NETWORK_APN is set in secrets.h)
// #define TINY_GSM_RX_BUFFER 1024
// #define LILYGO_T_A7670 //temp
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#ifdef LILYGO_T_A7670
// RGB macros
#define RGB_RED_PIN 16
#define RGB_GREEN_PIN 15
#define RGB_BLUE_PIN 7
#elif LILYGO_T_SIM7670G_S3
#define RGB_RED_PIN 13
#define RGB_GREEN_PIN 14
#define RGB_BLUE_PIN 21
#endif

// DHT22 macros
#define DHTTYPE DHT22
#ifdef LILYGO_T_A7670
#define DHT_PIN 32
#elif LILYGO_T_SIM7670G_S3
#define DHT_PIN 7
#endif
#define TEMP_DELTA_THRESHOLD 0.1f
#define HUM_DELTA_THRESHOLD 1.0f

// Macros for Polar H9
#define STRAP_NAME "POLAR H9 EC351E2B"
#define STRAP_ADDRESS "a0:9e:1a:ec:35:1e"
#define HEARTRATE_SERVICE_UUID "180D"
#define HEARTRATE_CHAR_UUID "2A37"

// Gas sensor macros
#ifdef LILYGO_T_A7670
#define GAS_PIN (32)
#elif LILYGO_T_SIM7670G_S3
#define GAS_PIN (15)
#endif
// #define GAS_BOARD               "ESP-32"
// #define GAS_TYPE                "MQ-2"
// #define GAS_VOLTAGE_RESOLUTION  12
// #define GAS_ADC_BIT_RESOLUTION  12
#define GAS_RATIO_CLEANAIR 9.83f
#define GAS_DELTA_THRESHOLD 5.0f
/*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
*/
#define GAS_SETA 658.71f
#define GAS_SETB -2.168f

// Accelerometer macros

// Setup I2C
#ifdef LILYGO_T_A7670
#define SDA_PIN 21
#define SCL_PIN 22
#elif LILYGO_T_SIM7670G_S3
#define SDA_PIN 6
#define SCL_PIN 5
#endif

#define MPU6500_ADDR 0x68

// Accelerometer fall thresholds
#define ACC_THRESHOLD 2.0f
#define ANGLE_THRESHOLD 60.0f
#define MAX_TIME_BETWEEN 2000

// Simple step detection
#define STEP_THRESHOLD 1.5f
#define STEP_MIN_TIME_MS 400

// Kalibrering
#define X_OFFSET 0.0737f
#define Y_OFFSET -0.6132f
#define Z_OFFSET -0.9986f

// Bluetooth (meh, can't find the correct flag to turn off verbose log)
#define CONFIG_NIMBLE_CPP_LOG_LEVEL 0

#endif
