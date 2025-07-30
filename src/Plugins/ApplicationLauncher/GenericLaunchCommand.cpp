#include "GenericLaunchCommand.h"
#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <filesystem>
#include <algorithm>

GenericLaunchCommand::GenericLaunchCommand() 
    : m_applicationFinder(std::make_unique<ApplicationFinder>()) {
}

std::wstring GenericLaunchCommand::GetName() const {
    return L"Launch Application";
}

std::wstring GenericLaunchCommand::GetDescription() const {
    return L"Launch any application by name or path. Example: 'launch spotify' or 'launch notepad'";
}

CommandCategory GenericLaunchCommand::GetCategory() const {
    return CommandCategory::APPLICATION_LAUNCHER;
}

void GenericLaunchCommand::Execute() {
    // Diese Methode wird für die Standardausführung verwendet
    // Der tatsächliche Launch-Prozess erfolgt über SetSearchTerm und LaunchApplication
    if (!m_currentSearchTerm.empty()) {
        auto applications = GetMatchingApplications();
        if (!applications.empty()) {
            // Starte die erste/beste Übereinstimmung
            LaunchApplication(applications[0]);
        }
    }
}

void GenericLaunchCommand::SetSearchTerm(const std::wstring& searchTerm) {
    m_currentSearchTerm = searchTerm;
}

std::vector<ApplicationInfo> GenericLaunchCommand::GetMatchingApplications() const {
    if (m_currentSearchTerm.empty()) {
        return {};
    }
    
    auto results = m_applicationFinder->FindApplications(m_currentSearchTerm);
    
    // Ensure all applications have icons loaded
    for (auto& app : results) {
        if (!app.hIcon) {
            // If icon is not loaded, try to load it now (non-const operation)
            // This is a workaround since we can't modify the finder here
        }
    }
    
    return results;
}

void GenericLaunchCommand::LaunchApplication(const ApplicationInfo& app) {
    std::thread([this, app]() {
        // Versuche zuerst den direkten Pfad
        if (LaunchByPath(app.path)) {
            return;
        }
        
        // Falls das fehlschlägt, versuche über den Namen
        LaunchByName(app.name);
    }).detach();
}

bool GenericLaunchCommand::LaunchByPath(const std::wstring& path) {
    std::wstring resolvedPath = ResolvePath(path);
    
    // Versuche ShellExecute
    HINSTANCE result = ShellExecuteW(NULL, L"open", resolvedPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    // ShellExecute gibt einen Wert > 32 zurück bei Erfolg
    return (reinterpret_cast<INT_PTR>(result) > 32);
}

bool GenericLaunchCommand::LaunchByName(const std::wstring& name) {
    // Versuche den Namen direkt als ausführbaren Befehl
    HINSTANCE result = ShellExecuteW(NULL, L"open", name.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    // ShellExecute gibt einen Wert > 32 zurück bei Erfolg
    return (reinterpret_cast<INT_PTR>(result) > 32);
}

std::wstring GenericLaunchCommand::ResolvePath(const std::wstring& input) {
    // Falls es ein vollständiger Pfad ist
    if (std::filesystem::exists(input)) {
        return input;
    }
    
    // Falls es nur ein Dateiname ist, suche in bekannten Verzeichnissen
    std::filesystem::path inputPath(input);
    if (!inputPath.has_parent_path()) {
        // Suche in System32
        std::wstring system32Path = L"C:\\Windows\\System32\\" + input;
        if (std::filesystem::exists(system32Path)) {
            return system32Path;
        }
        
        // Suche im Windows-Verzeichnis
        std::wstring windowsPath = L"C:\\Windows\\" + input;
        if (std::filesystem::exists(windowsPath)) {
            return windowsPath;
        }
    }
    
    // Gib den ursprünglichen Input zurück
    return input;
} 