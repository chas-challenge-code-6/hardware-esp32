#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

uint8_t const RGB_RED_PIN{16};
uint8_t const RGB_GREEN_PIN{15};
uint8_t const RGB_BLUE_PIN{7};

#define DHTTYPE DHT22
uint8_t const DHT_PIN{20};

#define MQ2
uint8_t const MQ2_PIN{34};

#define BATTERY
uint8_t const BOARD_BAT_ADC_PIN{35}; // ADC pin for battery voltage measurement
uint8_t const BOARD_POWERON_PIN{23}; // Power on pin for the board
uint8_t const BOARD_BATTERY_VOLTAGE_DIVIDER{2}; // Voltage divider for battery measurement

#endif
