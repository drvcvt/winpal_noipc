#include "OpenDownloadsCommand.h"
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

std::wstring OpenDownloadsCommand::GetName() const {
    return L"Open Downloads Folder";
}

std::wstring OpenDownloadsCommand::GetDescription() const {
    return L"Opens the user's Downloads folder.";
}

CommandCategory OpenDownloadsCommand::GetCategory() const {
    return CommandCategory::FILE_TOOLS;
}

void OpenDownloadsCommand::Execute() {
    wchar_t downloadsPath[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, downloadsPath) == S_OK) {
        std::wstring fullPath = std::wstring(downloadsPath) + L"\\Downloads";
        ShellExecuteW(NULL, L"open", fullPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    } else {
        // Fallback to generic Downloads folder
        ShellExecuteW(NULL, L"open", L"shell:Downloads", NULL, NULL, SW_SHOWNORMAL);
    }
} 