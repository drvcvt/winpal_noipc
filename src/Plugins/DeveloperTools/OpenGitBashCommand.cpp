#include "OpenGitBashCommand.h"
#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <thread>

std::wstring OpenGitBashCommand::GetName() const {
    return L"Open Git Bash";
}

std::wstring OpenGitBashCommand::GetDescription() const {
    return L"Opens Git Bash terminal (if Git is installed).";
}

CommandCategory OpenGitBashCommand::GetCategory() const {
    return CommandCategory::DEVELOPER_TOOLS;
}

void OpenGitBashCommand::Execute() {
    std::thread([]() {
        std::vector<std::wstring> gitBashPaths = {
            L"C:\\Program Files\\Git\\bin\\bash.exe",
            L"C:\\Program Files (x86)\\Git\\bin\\bash.exe",
            L"C:\\Users\\%USERNAME%\\AppData\\Local\\Programs\\Git\\bin\\bash.exe"
        };
        
        bool found = false;
        for (const auto& path : gitBashPaths) {
            if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                found = true;
                break;
            }
        }
        
        if (!found) {
            MessageBoxW(NULL, L"Git Bash not found. Please install Git for Windows.", L"Warning", MB_OK | MB_ICONWARNING);
        }
    }).detach();
} 