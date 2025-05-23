#ifndef MAIN_H
#define MAIN_H

#define DEBUG 1

// DEVICE_ID for API
#define DEVICE_ID "SENTINEL-001"

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
#define TEMP_DELTA_THRESHOLD 0.1
#define HUM_DELTA_THRESHOLD 1.0

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
// #define GAS_BOARD               ("ESP-32")
// #define GAS_TYPE                ("MQ-2")
// #define GAS_VOLTAGE_RESOLUTION  (12)
// #define GAS_ADC_BIT_RESOLUTION  (12)
#define GAS_RATIO_CLEANAIR         (9.83)
#define GAS_DELTA_THRESHOLD 5.0

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
#define ACC_THRESHOLD 2.0
#define ANGLE_THRESHOLD 60.0
#define MAX_TIME_BETWEEN 2000
#define STEP_THRESHOLD 1.2
#define STEP_DEBOUNCE_MS 300

// Kalibrering
#define X_OFFSET 0.0737
#define Y_OFFSET -0.6132
#define Z_OFFSET -0.9986

// Battery macros
#ifdef LILYGO_T_A7670
#define BOARD_BAT_ADC_PIN 35
#elif LILYGO_T_SIM7670G_S3
#define BOARD_BAT_ADC_PIN 4


#endif

#endif // MAIN_H