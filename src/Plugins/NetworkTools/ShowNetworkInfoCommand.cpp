#include "ShowNetworkInfoCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring ShowNetworkInfoCommand::GetName() const {
    return L"Show Network Information";
}

std::wstring ShowNetworkInfoCommand::GetDescription() const {
    return L"Opens a command prompt showing network configuration (ipconfig).";
}

CommandCategory ShowNetworkInfoCommand::GetCategory() const {
    return CommandCategory::NETWORK_TOOLS;
}

void ShowNetworkInfoCommand::Execute() {
    ShellExecuteW(NULL, L"open", L"cmd.exe", L"/k ipconfig /all", NULL, SW_SHOWNORMAL);
} 