#ifndef DEVICES_H
#define DEVICES_H

#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include <SwitchService.h>
#include <IRTransmitterService.h>

class Devices {

private:
    String deviceName;

public:

    Devices(String deviceName);

    void init();

    void blinkLed(int times, int interval);

    void handleMqttCommunication(EspMQTTClient& mqttClient);

    void connectionFailed();

    void activateOfflineMode();

    void getStatus(JsonObject& obj);

};

#endif