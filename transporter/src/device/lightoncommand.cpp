// lightoncommand.cpp
#include "lightoncommand.h"
#include "device/light.h"
LightOnCommand::LightOnCommand(std::shared_ptr<Light> light)
    : mLight(light) 
{
    
}

void LightOnCommand::execute() {
    mLevel = mLight->getLevel();
    mLight->on();
}

void LightOnCommand::undo() {

    mLight->dim(mLevel);
}