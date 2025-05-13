#include "network/lte.h"

LTEClient::LTEClient(Stream& stream) : modem(stream) {}

void LTEClient::begin() {
    Serial.println("Initializing LTE modem...");
    modem.restart();
    modem.gprsConnect("your_apn", "", "");
    Serial.println("Connected to LTE");
}

bool LTEClient::isConnected() {
    return modem.isNetworkConnected();
}
