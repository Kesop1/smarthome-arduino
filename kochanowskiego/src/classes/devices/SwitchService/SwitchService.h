#ifndef SWITCHSERVICE_H
#define SWISWITCHSERVICE_HTCH_H

#include "../ElementService/ElementService.h"
#include <Arduino.h>

class SwitchService: public ElementService {

private:
    boolean switched;
    byte pin;

public:

    SwitchService(byte pin, String name);

    void init();

    void on();

    void off();

    boolean isSwitched();

    boolean handleCommand(String command);

    void switchElement(boolean high);

    String getStatus();
};

#endif