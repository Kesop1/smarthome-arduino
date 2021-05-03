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