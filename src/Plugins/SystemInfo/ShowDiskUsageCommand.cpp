#include "ShowDiskUsageCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring ShowDiskUsageCommand::GetName() const {
    return L"Show Disk Usage";
}

std::wstring ShowDiskUsageCommand::GetDescription() const {
    return L"Opens the Windows Disk Cleanup utility to show disk usage.";
}

CommandCategory ShowDiskUsageCommand::GetCategory() const {
    return CommandCategory::SYSTEM_INFO;
}

void ShowDiskUsageCommand::Execute() {
    ShellExecuteW(NULL, L"open", L"cleanmgr.exe", NULL, NULL, SW_SHOWNORMAL);
} 