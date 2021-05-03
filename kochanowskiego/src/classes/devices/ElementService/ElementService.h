#ifndef ELEMENTSERVICE_H
#define ELEMENTSERVICE_H

#include <Arduino.h>

class ElementService {

private:
    String name;

public:
    ElementService(String name);

    String getName();

    virtual boolean handleCommand(String command) = 0;

    virtual String getStatus() = 0;
};

#endif