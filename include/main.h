#ifndef MAIN_H
#define MAIN_H

#define DEBUG 1

// DEVICE_ID for API
#define DEVICE_ID "SENTINEL-001";

// RGB macros
#define RGB_RED_PIN 16
#define RGB_GREEN_PIN 15
#define RGB_BLUE_PIN 7

// DHT22 macros
#define DHTTYPE DHT22
#define DHT_PIN 32

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

// Accelerometer macros
// Setup I2C
#define SDA_PIN 21
#define SCL_PIN 22

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

#endif
