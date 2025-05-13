#ifndef LTE_H
#define LTE_H

#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>

class LTEClient
{
public:
    LTEClient(Stream &stream);
    void begin();
    bool isConnected();

private:
    TinyGsm modem;
};

#endif
