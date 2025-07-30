#include "PingCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring PingCommand::GetName() const {
    return L"Ping Google DNS";
}

std::wstring PingCommand::GetDescription() const {
    return L"Pings Google's DNS server (8.8.8.8) to test internet connectivity.";
}

CommandCategory PingCommand::GetCategory() const {
    return CommandCategory::NETWORK_TOOLS;
}

void PingCommand::Execute() {
    ShellExecuteW(NULL, L"open", L"cmd.exe", L"/k ping 8.8.8.8", NULL, SW_SHOWNORMAL);
} 