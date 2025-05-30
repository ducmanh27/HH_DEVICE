// lightoncommand.h
#ifndef LIGHTONCOMMAND_H
#define LIGHTONCOMMAND_H

#include "command.h"
#include <memory>
class Light;
class LightOnCommand : public Command {
public:
    explicit LightOnCommand(std::shared_ptr<Light> light);
    void execute() override;
    void undo() override;
    
private:
    std::shared_ptr<Light> mLight;
    uint8_t mLevel {0}; // 0-255 for PWM
};

#endif // LIGHTONCOMMAND_H
