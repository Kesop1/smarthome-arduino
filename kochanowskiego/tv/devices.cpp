#include "devices.h"

String LED_SWITCH_NAME = "ledSwitch";
SwitchService ledSwitch(LED_BUILTIN, LED_SWITCH_NAME);

String TV_SWITCH_NAME = "tvSwitch";
SwitchService tvSwitch(D5, TV_SWITCH_NAME);

String AMP_SWITCH_NAME = "ampSwitch";
SwitchService ampSwitch(D6, AMP_SWITCH_NAME);

String SUBWOOFER_SWITCH_NAME = "subwooferSwitch";
SwitchService subwooferSwitch(D7, SUBWOOFER_SWITCH_NAME);

String PS3_SWITCH_NAME = "ps3Switch";
SwitchService ps3Switch(D8, PS3_SWITCH_NAME);

String IR_TRANSMITTER_NAME = "irTransmitter";
IRTransmitterService irTransmitter(D2, IR_TRANSMITTER_NAME);

Devices::Devices(String deviceName) {
    this->deviceName = deviceName;
}

void Devices::init() {
    ledSwitch.init();
    tvSwitch.init();
    ampSwitch.init();
    subwooferSwitch.init();
    ps3Switch.init();
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

void Devices::connectionFailed() {
  blinkLed(2, 100);
  blinkLed(2, 300);
}

void Devices::activateOfflineMode() {
  blinkLed(3, 100);
  blinkLed(3, 300);
  blinkLed(3, 100);
  ledSwitch.on();
  tvSwitch.on();
  ampSwitch.on();
  subwooferSwitch.on();
  ps3Switch.on();
}

void Devices::handleMqttCommunication(EspMQTTClient& mqttClient) {
    ledSwitch.handleMqtt(mqttClient, deviceName);
    tvSwitch.handleMqtt(mqttClient, deviceName);
    ampSwitch.handleMqtt(mqttClient, deviceName);
    subwooferSwitch.handleMqtt(mqttClient, deviceName);
    ps3Switch.handleMqtt(mqttClient, deviceName);
    irTransmitter.handleMqtt(mqttClient, deviceName);
}

void Devices::getStatus(JsonObject& obj) {
  Serial.println("Reading Devices status");
  JsonObject ledSwitchStatus = obj.createNestedObject(LED_SWITCH_NAME);
  ledSwitchStatus["status"] = ledSwitch.getStatus();

  JsonObject tvSwitchStatus = obj.createNestedObject(TV_SWITCH_NAME);
  tvSwitchStatus["status"] = tvSwitch.getStatus();

  JsonObject ampSwitchStatus = obj.createNestedObject(AMP_SWITCH_NAME);
  ampSwitchStatus["status"] = ampSwitch.getStatus();

  JsonObject subwooferSwitchStatus = obj.createNestedObject(SUBWOOFER_SWITCH_NAME);
  subwooferSwitchStatus["status"] = subwooferSwitch.getStatus();

  JsonObject ps3SwitchStatus = obj.createNestedObject(PS3_SWITCH_NAME);
  ps3SwitchStatus["status"] = ps3Switch.getStatus();

  JsonObject irTransmitterStatus = obj.createNestedObject(IR_TRANSMITTER_NAME);
  irTransmitterStatus["status"] = irTransmitter.getStatus();
}