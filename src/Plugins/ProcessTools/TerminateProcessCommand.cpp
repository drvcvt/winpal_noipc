#include "TerminateProcessCommand.h"
#include <windows.h>

std::wstring TerminateProcessCommand::GetName() const {
    return L"Terminate Process";
}

std::wstring TerminateProcessCommand::GetDescription() const {
    return L"Terminates a selected process.";
}

CommandCategory TerminateProcessCommand::GetCategory() const {
    return CommandCategory::PROCESS_TOOLS;
}

void TerminateProcessCommand::Execute() {
    MessageBoxW(NULL, L"Terminate Process command executed (Not implemented yet).", L"Info", MB_OK);
}
