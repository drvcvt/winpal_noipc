#include "LaunchCalculatorCommand.h"
#include <windows.h>
#include <shellapi.h>
#include <thread>

std::wstring LaunchCalculatorCommand::GetName() const {
    return L"Calculator";
}

std::wstring LaunchCalculatorCommand::GetDescription() const {
    return L"Opens the Windows Calculator application.";
}

CommandCategory LaunchCalculatorCommand::GetCategory() const {
    return CommandCategory::APPLICATION_LAUNCHER;
}

void LaunchCalculatorCommand::Execute() {
    std::thread([]() {
        ShellExecuteW(NULL, L"open", L"calc.exe", NULL, NULL, SW_SHOWNORMAL);
    }).detach();
} 