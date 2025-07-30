#include "ShowSystemInfoCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring ShowSystemInfoCommand::GetName() const {
    return L"Show System Information";
}

std::wstring ShowSystemInfoCommand::GetDescription() const {
    return L"Opens the Windows System Information utility.";
}

CommandCategory ShowSystemInfoCommand::GetCategory() const {
    return CommandCategory::SYSTEM_INFO;
}

void ShowSystemInfoCommand::Execute() {
    ShellExecuteW(NULL, L"open", L"msinfo32.exe", NULL, NULL, SW_SHOWNORMAL);
} 