#include "sensors.h"

PhotoresistorService photoresistorService(A0);
DhtService dhtService;

Sensors::Sensors(String deviceName) {
    this->deviceName = deviceName;
}

void Sensors::init() {
  dhtService.init();
  photoresistorService.init();
}

void Sensors::getSensorsReadings() {
  Serial.println("Reading Sensors");
  dhtService.getReadings();
  photoresistorService.getReadings();
}

void Sensors::handleMqttCommunication(EspMQTTClient& mqttClient) {
    dhtService.handleMqtt(mqttClient, deviceName);
    photoresistorService.handleMqtt(mqttClient, deviceName);
}

void Sensors::getStatus(JsonObject& obj) {
  JsonObject photoresistorStatus = obj.createNestedObject("photoresistor");
  photoresistorStatus["status"] = photoresistorService.getStatus();

  JsonObject dhtStatus = obj.createNestedObject("DHT");
  dhtStatus["status"] = dhtService.getStatus();
}

// TODO: errors