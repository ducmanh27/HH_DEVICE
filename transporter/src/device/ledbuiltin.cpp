#include "ledbuiltin.h"

LedBuiltin::LedBuiltin(uint8_t pin) : mPin(pin) {
    pinMode(mPin, OUTPUT);
    off();
}

void LedBuiltin::on() {
    digitalWrite(mPin, HIGH);
    setLevel(100);
}

void LedBuiltin::off() {
    digitalWrite(mPin, LOW);
    setLevel(0); 
}


void LedBuiltin::dim(uint8_t level) {
    if (level == 0) {
        off();
        return;
    }
    level = (level > 100) ? 100 : level;
    if (level == 100) {
        on();
        return;
    }
    setLevel(level);
    int pwmValue = map(level, 0, 100, 0, 255);
    analogWrite(mPin, pwmValue);
}
