#include "devices.h"

String LED_SWITCH_NAME = "ledSwitch";
SwitchService ledSwitch(LED_BUILTIN, LED_SWITCH_NAME);

String IR_TRANSMITTER_NAME = "irTransmitter";
IRTransmitterService irTransmitter(D2, IR_TRANSMITTER_NAME);

Devices::Devices(String deviceName) {
    this->deviceName = deviceName;
}

void Devices::init() {
    ledSwitch.init();
    irTransmitter.init();
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
    ledSwitch.handleMqtt(mqttClient, deviceName);
    irTransmitter.handleMqtt(mqttClient, deviceName);
}

void Devices::getStatus(JsonObject& obj) {
  JsonObject ledSwitchStatus = obj.createNestedObject(LED_SWITCH_NAME);
  ledSwitchStatus["status"] = ledSwitch.getStatus();

  JsonObject irTransmitterStatus = obj.createNestedObject(IR_TRANSMITTER_NAME);
  irTransmitterStatus["status"] = irTransmitter.getStatus();
}