#include "light.h"
#include <Arduino.h>

uint8_t Light::getLevel() const {
    return mLevel;
}

void Light::setLevel(uint8_t level) {
    mLevel = level;
}