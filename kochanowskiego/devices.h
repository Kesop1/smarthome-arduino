#ifndef DEVICES_H
#define DEVICES_H

#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "src/classes/devices/SwitchService/SwitchService.h"
#include "src/classes/devices/IRTransmitterService/IRTransmitterService.h"

class Devices {

private:
    String deviceName;

public:

    Devices(String deviceName);

    void init();

    void blinkLed(int times, int interval);

    void handleMqttCommunication(EspMQTTClient& mqttClient);

    void activateOfflineMode();

    void getStatus(JsonObject& obj);

};

#endif