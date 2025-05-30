#ifndef LIGHTOFFCOMMAND_H
#define LIGHTOFFCOMMAND_H

#include "command.h"
#include <memory>
class Light;

class LightOffCommand : public Command {
public:
    explicit LightOffCommand(std::shared_ptr<Light> light);
    void execute() override;
    void undo() override;

private:
    std::shared_ptr<Light> mLight;
    uint8_t mLevel {0}; // lưu trạng thái trước khi tắt
};

#endif // LIGHTOFFCOMMAND_H