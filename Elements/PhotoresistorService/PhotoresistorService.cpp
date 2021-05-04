#include "PhotoresistorService.h"

/*
* DHT11 SENSOR:

      L     R
      |     |
     DATA   |
      |     |
    10k    VCC
   |  
  GND
*/

// #define PIN A0

int value = 0;

PhotoresistorService::PhotoresistorService(uint8_t pin) {
  this->pin = pin;
}

void PhotoresistorService::init() {
  getReadings();
}

void PhotoresistorService::handleMqtt(EspMQTTClient& mqttClient, String deviceName) {
  String dhtMqttTopic = deviceName + "/" + "photoresistor";

  mqttClient.subscribe(dhtMqttTopic.c_str(), [this, &mqttClient, dhtMqttTopic] (const String &payload)  {
      Serial.println("Photoresistor command: " + payload);
      if (getReadings()) {
        mqttClient.publish((dhtMqttTopic + "/value").c_str(), String(value), true);
      }
  });
}

boolean PhotoresistorService::getReadings() {
  Serial.println("Reading Photoresistor sensor");
  value = analogRead(pin);
  Serial.print(F("Photorestistance: "));
  Serial.println(value);
  return true;
}

String PhotoresistorService::getStatus() {
  if(value >= 0) {
    return "OK";
  } else {
    return "READ_ERROR";
  }
}

// TODO: errors