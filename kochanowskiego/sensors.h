#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "src/classes/sensors/dht/dht_service.h"
#include "src/classes/sensors/photoresistor/photoresistor_service.h"

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