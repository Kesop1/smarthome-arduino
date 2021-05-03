#include "ElementService.h"

ElementService::ElementService(String name) {
    this->name = name;
}

String ElementService::getName() {
    return name;
}