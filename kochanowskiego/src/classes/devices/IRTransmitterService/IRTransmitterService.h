#ifndef IRTRANSMITTERSERVICE_H
#define IRTRANSMITTERSERVICE_H

#include "../ElementService/ElementService.h"
#include <Arduino.h>
#include <EspMQTTClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

class IRTransmitterService: public ElementService {

private:
    uint8_t pin;

public:
    IRTransmitterService(uint8_t pin, String name);

    void init();

    void handleMqtt(EspMQTTClient& mqttClient, String deviceName);

    boolean handleCommand(String command);

    String getStatus();
};

#endif