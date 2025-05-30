#include "lightoffcommand.h"
#include "device/light.h"

LightOffCommand::LightOffCommand(std::shared_ptr<Light> light)
    : mLight(light)
{
}

void LightOffCommand::execute() {
    mLevel = mLight->getLevel();
    mLight->off();
}

void LightOffCommand::undo() {
    mLight->dim(mLevel); // phục hồi độ sáng cũ
}