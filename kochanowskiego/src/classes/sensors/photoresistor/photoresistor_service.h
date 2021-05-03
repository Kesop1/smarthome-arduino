#ifndef PHOTORESISTORSERVICE_H
#define PHOTORESISTORSERVICE_H

#include <Arduino.h>
#include <EspMQTTClient.h>

class PhotoresistorService {

private:

    uint8_t pin;

public:

    PhotoresistorService(uint8_t pin);

    void init();

    void handleMqtt(EspMQTTClient& mqttClient, String deviceName);

    boolean getReadings();

    String getStatus();

};

#endif