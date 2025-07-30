#include "OpenDesktopCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring OpenDesktopCommand::GetName() const {
    return L"Open Desktop Folder";
}

std::wstring OpenDesktopCommand::GetDescription() const {
    return L"Opens the user's Desktop folder.";
}

CommandCategory OpenDesktopCommand::GetCategory() const {
    return CommandCategory::FILE_TOOLS;
}

void OpenDesktopCommand::Execute() {
    ShellExecuteW(NULL, L"open", L"shell:Desktop", NULL, NULL, SW_SHOWNORMAL);
} 