#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include <DhtService.h>
#include <PhotoresistorService.h>

class Sensors {

private:

    String deviceName;

public:
    Sensors(String deviceName);

    void init();

    void handleMqttCommunication(EspMQTTClient& mqttClient);

    void getSensorsReadings();

    void getStatus(JsonObject& obj);

};

#endif