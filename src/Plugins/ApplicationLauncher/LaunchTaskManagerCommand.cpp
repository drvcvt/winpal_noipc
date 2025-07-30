#include "LaunchTaskManagerCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring LaunchTaskManagerCommand::GetName() const {
    return L"Task Manager";
}

std::wstring LaunchTaskManagerCommand::GetDescription() const {
    return L"Opens the Windows Task Manager.";
}

CommandCategory LaunchTaskManagerCommand::GetCategory() const {
    return CommandCategory::APPLICATION_LAUNCHER;
}

void LaunchTaskManagerCommand::Execute() {
    // Direkter Aufruf ohne Thread - ShellExecuteW ist bereits asynchron
    ShellExecuteW(NULL, L"open", L"taskmgr.exe", NULL, NULL, SW_SHOWNORMAL);
} 