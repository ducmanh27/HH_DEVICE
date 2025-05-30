// ledbuiltin.h
#ifndef LEDBUILTIN_H
#define LEDBUILTIN_H
#include "light.h"
#include <Arduino.h>

class LedBuiltin : public Light {

public:
    explicit LedBuiltin(uint8_t pin = 2);
    void on() override;
    void off() override;
    void dim(uint8_t level) override;
private:
    uint8_t mPin;
};

#endif // LEDBUILTIN_H

