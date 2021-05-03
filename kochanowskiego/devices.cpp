#include "devices.h"

SwitchService ledSwitch(LED_BUILTIN, "ledSwitch");
IRTransmitterService irTransmitterService(D2, "irTransmitter");

Devices::Devices(String deviceName) {
    this->deviceName = deviceName;
}

void Devices::init() {
    ledSwitch.init();
    irTransmitterService.init();
}

void Devices::blinkLed(int times, int interval) {
  for(int i = 0; i < times; i++) {
    ledSwitch.on();
    delay(interval);
    ledSwitch.off();
    delay(interval);
  }
  ledSwitch.off();
}


void Devices::activateOfflineMode() {
  blinkLed(3, 100);
  blinkLed(3, 300);
  blinkLed(3, 100);
}

void Devices::handleMqttCommunication(EspMQTTClient& mqttClient) {
    String ledSwitchMqttTopic = deviceName + "/" + ledSwitch.getName();
    String ledSwitchMqttStatusTopic = ledSwitchMqttTopic + "/status";
    //get previous state
    mqttClient.subscribe(ledSwitchMqttStatusTopic, [this, &mqttClient, ledSwitchMqttStatusTopic] (const String &payload)  {
        Serial.println(ledSwitch.getName() + " initial status: " + payload);
        ledSwitch.handleCommand(payload);
        mqttClient.unsubscribe(ledSwitchMqttStatusTopic.c_str());
    });

    mqttClient.subscribe(ledSwitchMqttTopic.c_str(), [this, &mqttClient, ledSwitchMqttStatusTopic] (const String &payload)  {
        Serial.println(ledSwitch.getName() + " command: " + payload);
        if (ledSwitch.handleCommand(payload)) {
            mqttClient.publish(ledSwitchMqttStatusTopic.c_str(), ledSwitch.getStatus(), true);
        }
    });

    irTransmitterService.handleMqtt(mqttClient, deviceName);
}

void Devices::getStatus(JsonObject& obj) {
  JsonObject ledSwitchStatus = obj.createNestedObject("ledSwitch");
  ledSwitchStatus["status"] = ledSwitch.getStatus();

  JsonObject irTransmitterStatus = obj.createNestedObject("IRTransmitter");
  irTransmitterStatus["status"] = irTransmitterService.getStatus();
}