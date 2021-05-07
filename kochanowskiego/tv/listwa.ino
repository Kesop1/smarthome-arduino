#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include "devices.h"
#include "sensors.h"

const String DEVICE_NAME = "listwaTv";

const char* WIFI_SSID     = "TP-LINK_5DBD15";
const char* WIFI_PASSWORD = "Qweasdzxc1";

const char* MQTT_SERVER   = "192.168.1.101";
const int MQTT_PORT       = 1883;

int failedConectionAttempts = 0;

long currentMillis;
long previousSensorsReadMillis = 0;
const long sensorsReadInterval = 30000;
long previousStatusUpdateMillis = 0;
const long statusUpdateInterval = 60000;

const String MQTT_TOPIC_STATUS = "status";
const String DEVICE_STATUS_MQTT_TOPIC = DEVICE_NAME + "/" + MQTT_TOPIC_STATUS;

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
  currentMillis = millis();
  if (currentMillis - previousSensorsReadMillis > sensorsReadInterval) {
    previousSensorsReadMillis = currentMillis;
    sensors.getSensorsReadings();
  }
  if (currentMillis - previousStatusUpdateMillis > statusUpdateInterval) {
    previousStatusUpdateMillis = currentMillis;
    sendStatusUpdate();
  }
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
  mqttClient.publish(DEVICE_STATUS_MQTT_TOPIC.c_str(), "connected");
  mqttClient.subscribe(DEVICE_NAME.c_str(), [DEVICE_NAME, &mqttClient] (const String &payload)  {
      Serial.println(DEVICE_NAME + " command: " + payload);
      if (payload == "STATUS") {
          sendStatusUpdate();
      }
  });
}

void sendStatusUpdate() {
  String status = getStatus();
  mqttClient.publish(DEVICE_STATUS_MQTT_TOPIC.c_str(), status, true);
}

String getStatus() {
  String status;
  StaticJsonDocument<800> doc;
  JsonObject deviceStatus = doc.createNestedObject(DEVICE_NAME);
  deviceStatus["status"] = "OK";
  JsonObject sensorsStatus = deviceStatus.createNestedObject("sensors");
  sensors.getStatus(sensorsStatus);
  JsonObject devicesStatus = deviceStatus.createNestedObject("devices");
  devices.getStatus(devicesStatus);
  serializeJson(doc, status);
  Serial.print("Status message uses: ");
  Serial.print(doc.memoryUsage());
  Serial.print(" of available: ");
  Serial.println(doc.capacity());
  Serial.println(status);//TODO: message too large for MQTT
  return status;
}

void activateOfflineMode() {
  Serial.println("Device is offline!");
  devices.activateOfflineMode();
}
