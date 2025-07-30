#include "LaunchNotepadCommand.h"
#include <windows.h>
#include <shellapi.h>
#include <thread>

std::wstring LaunchNotepadCommand::GetName() const {
    return L"Notepad";
}

std::wstring LaunchNotepadCommand::GetDescription() const {
    return L"Opens the Windows Notepad text editor.";
}

CommandCategory LaunchNotepadCommand::GetCategory() const {
    return CommandCategory::APPLICATION_LAUNCHER;
}

void LaunchNotepadCommand::Execute() {
    std::thread([]() {
        ShellExecuteW(NULL, L"open", L"notepad.exe", NULL, NULL, SW_SHOWNORMAL);
    }).detach();
} 