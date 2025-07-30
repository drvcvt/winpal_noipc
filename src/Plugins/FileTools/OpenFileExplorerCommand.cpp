#include "OpenFileExplorerCommand.h"
#include <windows.h>
#include <shellapi.h>

std::wstring OpenFileExplorerCommand::GetName() const {
    return L"Open File Explorer";
}

std::wstring OpenFileExplorerCommand::GetDescription() const {
    return L"Opens Windows File Explorer at the default location.";
}

CommandCategory OpenFileExplorerCommand::GetCategory() const {
    return CommandCategory::FILE_TOOLS;
}

void OpenFileExplorerCommand::Execute() {
    ShellExecuteW(NULL, L"open", L"explorer.exe", NULL, NULL, SW_SHOWNORMAL);
} 