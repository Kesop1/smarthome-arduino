#include "IRTransmitterService.h"

const uint16_t kIrLed = 4;

IRsend irsend(kIrLed);

IRTransmitterService::IRTransmitterService(uint8_t pin, String name) 
    : ElementService(name) {
    this->pin = pin;
    init();
}

void IRTransmitterService::init() {
  irsend.begin();
}

void IRTransmitterService::handleMqtt(EspMQTTClient& mqttClient, String deviceName) {
  String irTransmitterMqttTopic = deviceName + "/" + "ir_transmitter";

  mqttClient.subscribe(irTransmitterMqttTopic.c_str(), [this, &mqttClient, irTransmitterMqttTopic] (const String &payload)  {
      Serial.println("IRTransmitter command: " + payload);
      handleCommand(payload);
  });
}

boolean IRTransmitterService::handleCommand(String command) { //TODO: handle commands
  Serial.println(getName() + " command received: " + command);
  irsend.sendNEC(0xFFFFFFFF, 32);
  return false;
}

String IRTransmitterService::getStatus() {
  return "OK";
}