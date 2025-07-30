#include "OpenProcessPathCommand.h"
#include <windows.h>

std::wstring OpenProcessPathCommand::GetName() const {
    return L"Open Process Path";
}

std::wstring OpenProcessPathCommand::GetDescription() const {
    return L"Opens the file location of a selected process.";
}

CommandCategory OpenProcessPathCommand::GetCategory() const {
    return CommandCategory::PROCESS_TOOLS;
}

void OpenProcessPathCommand::Execute() {
    MessageBoxW(NULL, L"Open Process Path command executed (Not implemented yet).", L"Info", MB_OK);
}
