#ifndef DEVICE_COMMAND_H
#define DEVICE_COMMAND_H
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};
#endif // DEVICE_COMMAND_H