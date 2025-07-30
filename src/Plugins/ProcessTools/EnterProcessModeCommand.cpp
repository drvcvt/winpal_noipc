#include "EnterProcessModeCommand.h"
#include <windows.h>

std::wstring EnterProcessModeCommand::GetName() const {
    return L"Enter Process Mode";
}

std::wstring EnterProcessModeCommand::GetDescription() const {
    return L"Lists running processes to terminate or inspect them.";
}

CommandCategory EnterProcessModeCommand::GetCategory() const {
    return CommandCategory::PROCESS_TOOLS;
}

void EnterProcessModeCommand::Execute() {
    MessageBoxW(NULL, L"Process Mode activated (Not implemented yet).", L"Info", MB_OK);
}
