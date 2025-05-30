#pragma once
#include <stdint.h>
class Light {
public:
    virtual ~Light() = default;
    virtual void on() = 0;
    virtual void off() = 0;
    virtual void dim(uint8_t level) = 0; 

    uint8_t getLevel() const;
    void setLevel(uint8_t level);
private:
    uint8_t mLevel {0}; 
};