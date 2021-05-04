#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "devices.h"
#include "sensors.h"

const String DEVICE_NAME = "listwa_testowa";

const char* WIFI_SSID     = "TP-LINK_5DBD15";
const char* WIFI_PASSWORD = "Qweasdzxc1";

const char* MQTT_SERVER   = "192.168.1.101";
const int MQTT_PORT       = 1883;

int failedConectionAttempts = 0;

const String MQTT_TOPIC_STATUS = "status";

EspMQTTClient mqttClient(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, DEVICE_NAME.c_str(), MQTT_PORT);

Devices devices(DEVICE_NAME);
Sensors sensors(DEVICE_NAME);

void setUpEspMQTTClient() {
  // mqttClient.enableLastWillMessage((String(DEVICE_NAME) + "/" + MQTT_TOPIC_STATUS).c_str(), "disconnected", true);
}

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Hello from: " + DEVICE_NAME);
  setUpEspMQTTClient();
  devices.init();
  sensors.init();
}

void loop() {
  handleMQTTCommunication();
}

void handleMQTTCommunication() {
  mqttClient.loop();
  if(!mqttClient.isConnected()) {
    Serial.print("Connecting... ");
    Serial.print("WiFi: " + String(mqttClient.isWifiConnected()));
    Serial.println(", MQTT: " + String(mqttClient.isMqttConnected()));
    failedConectionAttempts++;
    devices.connectionFailed();
  } else {
    if(failedConectionAttempts > 0) {
      Serial.println("Connected");
    }
    failedConectionAttempts = 0;
  }
  if(failedConectionAttempts == 10) {
    activateOfflineMode();
  }
}

void onConnectionEstablished() {
  handleMqttCommunication();
  devices.handleMqttCommunication(mqttClient);
  sensors.handleMqttCommunication(mqttClient);
  sensors.getSensorsReadings();
}

void handleMqttCommunication() {
  String deviceStatusMqttTopic = DEVICE_NAME + "/" + MQTT_TOPIC_STATUS;
  mqttClient.publish(deviceStatusMqttTopic.c_str(), "connected");
  mqttClient.subscribe(DEVICE_NAME.c_str(), [DEVICE_NAME, &mqttClient, deviceStatusMqttTopic] (const String &payload)  {
      Serial.println(DEVICE_NAME + " command: " + payload);
      if (payload == "STATUS") {
          String status = getStatus();
          mqttClient.publish(deviceStatusMqttTopic.c_str(), status, true);
      }
  });
}

String getStatus() {
  String status;
  StaticJsonDocument<200> doc;
  JsonObject deviceStatus = doc.createNestedObject("status");
  deviceStatus["status"] = "OK";
  JsonObject devicesStatus = doc.createNestedObject("devices");
  devices.getStatus(devicesStatus);
  JsonObject sensorsStatus = doc.createNestedObject("sensors");
  sensors.getStatus(sensorsStatus);
  serializeJson(doc, status);
  return status;
}

void activateOfflineMode() {
  Serial.println("Device is offline!");
  devices.activateOfflineMode();
}
