#include "IRTransmitterService.h"

const uint16_t kIrLed = 4;

IRsend irsend(kIrLed);

String PROTOCOL_SAMSUNG = "SAMSUNG";

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

boolean IRTransmitterService::handleCommand(String command) {
  Serial.println(getName() + " command received: " + command);
  StaticJsonDocument<200> cmd;
  deserializeJson(cmd, command);

  int repeat = cmd["repeat"];
  String protocol = cmd["protocol"];
  int32_t signal = strtoul(cmd["signal"], NULL, 16);

  for(int i = 0; i<repeat; i++){
    if(PROTOCOL_SAMSUNG.equalsIgnoreCase(protocol)){
      // int32_t signal = strtoul((char*)msg.c_str() , NULL, 16 );
      irsend.sendSAMSUNG(signal, 32);
      delay(40);
    } else {
      // long signal = strtol((char*)msg.c_str(), NULL, 16);
      irsend.sendNEC(signal, 32);
      delay(100);
      irsend.sendNEC(0xFFFFFFFF, 32);
      delay(40);
    }
  }
  return true;
}

String IRTransmitterService::getStatus() {
  return "OK";
}