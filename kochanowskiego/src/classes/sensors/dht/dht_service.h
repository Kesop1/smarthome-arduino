#ifndef DHTSERVICE_H
#define DHTSERVICE_H

#include <Arduino.h>
#include <EspMQTTClient.h>
#include <DHT.h>

class DhtService {

public:

    DhtService();

    void init();

    void handleMqtt(EspMQTTClient& mqttClient, String deviceName);

    boolean getReadings();

    String getStatus();

};

#endif