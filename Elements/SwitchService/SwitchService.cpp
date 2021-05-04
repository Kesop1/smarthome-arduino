#include "SwitchService.h"

//TODO: Low switch

SwitchService::SwitchService(byte pin, String name) 
    : ElementService(name) {
    this->pin = pin;
    init();
}

void SwitchService::init() {
  pinMode(pin, OUTPUT);
}

void SwitchService::on() {
  digitalWrite(pin, LOW);
  switched = true;
}

void SwitchService::off() {
  digitalWrite(pin, HIGH);
  switched = false;
}

boolean SwitchService::isSwitched() {
    return switched;
}

void SwitchService::handleMqtt(EspMQTTClient& mqttClient, String deviceName) {
    String switchMqttTopic = deviceName + "/" + getName();
    String switchMqttStatusTopic = switchMqttTopic + "/status";
    //get previous state
    mqttClient.subscribe(switchMqttStatusTopic, [this, &mqttClient, switchMqttStatusTopic] (const String &payload)  {
        Serial.println(getName() + " initial status: " + payload);
        handleCommand(payload);
        mqttClient.unsubscribe(switchMqttStatusTopic.c_str());
    });

    mqttClient.subscribe(switchMqttTopic.c_str(), [this, &mqttClient, switchMqttStatusTopic] (const String &payload)  {
        Serial.println(getName() + " command: " + payload);
        if (handleCommand(payload)) {
            mqttClient.publish(switchMqttStatusTopic.c_str(), getStatus(), true);
        }
    });
}

boolean SwitchService::handleCommand(String command) {
  Serial.println(getName() + " command received: " + command);
  if(command == "ON") {
    switchElement(true);
    return true;
  } else if (command == "OFF") {
    switchElement(false);
    return true;
  }
  return false;
}

void SwitchService::switchElement(boolean high) {
  high ? on() : off();
}

String SwitchService::getStatus() {
  return "OK";
}