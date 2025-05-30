#include <memory>
#include <stack>
#include "commandmanager.h"
#include "command.h"

void CommandManager::executeCommand(std::shared_ptr<Command> cmd) {
    cmd->execute();
    undoStack.push(cmd);
    // Clear redo stack on new command
    while (!redoStack.empty()) redoStack.pop();
}

void CommandManager::undo() {
    if (!undoStack.empty()) {
        auto cmd = undoStack.top();
        undoStack.pop();
        cmd->undo();
        redoStack.push(cmd);
    }
}

void CommandManager::redo() {
    if (!redoStack.empty()) {
        auto cmd = redoStack.top();
        redoStack.pop();
        cmd->execute();
        undoStack.push(cmd);
    }
}
