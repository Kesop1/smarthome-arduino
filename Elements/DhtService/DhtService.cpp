#include "DhtService.h"

/*
* DHT11:
* VCC DATA  X GND
*/

#define DHTPIN D3 //TODO: define pin
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

float humidity;
float temperature;
float heatIndex;
String error = "";

DhtService::DhtService() {}

void DhtService::init() {
  dht.begin();
  getReadings();
}

void DhtService::handleMqtt(EspMQTTClient& mqttClient, String deviceName) {
  String dhtMqttTopic = deviceName + "/" + "dht";

  mqttClient.subscribe(dhtMqttTopic.c_str(), [this, &mqttClient, dhtMqttTopic] (const String &payload)  {
      Serial.println("DHT command: " + payload);
      if (getReadings()) {
        mqttClient.publish((dhtMqttTopic + "/temperature").c_str(), String(temperature), true);
        mqttClient.publish((dhtMqttTopic + "/humidity").c_str(), String(humidity), true);
        mqttClient.publish((dhtMqttTopic + "/heat_index").c_str(), String(heatIndex), true);
      }
  });
}

boolean DhtService::getReadings() {
  Serial.println("Reading DHT11 sensor");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    error = "READ_ERROR";
    return false;
  }
  float hic = dht.computeHeatIndex(t, h, false);
  heatIndex = hic;
  temperature = t;
  humidity = h;
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C  Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));
  error = "";
  return true;
}

String DhtService::getStatus() {
  if(error == "") {
    return "OK";
  } else {
    return error;
  }
}

// TODO: errors