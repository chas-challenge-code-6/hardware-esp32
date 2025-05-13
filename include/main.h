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

#endif
