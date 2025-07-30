#include "OpenPowerShellCommand.h"
#include <windows.h>
#include <shellapi.h>
#include <thread>

std::wstring OpenPowerShellCommand::GetName() const {
    return L"Open PowerShell";
}

std::wstring OpenPowerShellCommand::GetDescription() const {
    return L"Opens Windows PowerShell terminal.";
}

CommandCategory OpenPowerShellCommand::GetCategory() const {
    return CommandCategory::DEVELOPER_TOOLS;
}

void OpenPowerShellCommand::Execute() {
    std::thread([]() {
        ShellExecuteW(NULL, L"open", L"powershell.exe", NULL, NULL, SW_SHOWNORMAL);
    }).detach();
} 