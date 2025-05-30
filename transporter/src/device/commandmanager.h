// commandmanager.h
#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H
#include <memory>
#include <stack>

class Command;
class CommandManager {
public:
    void executeCommand(std::shared_ptr<Command> cmd);
    void undo();
    void redo();

private:
    std::stack<std::shared_ptr<Command>> undoStack;
    std::stack<std::shared_ptr<Command>> redoStack;
};

#endif // COMMAND_MANAGER_H
