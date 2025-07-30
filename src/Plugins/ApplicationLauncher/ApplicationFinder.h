#pragma once

#include <vector>
#include <string>
#include <memory>
#include <windows.h>

struct ApplicationInfo {
    std::wstring name;
    std::wstring path;
    std::wstring description;
    std::wstring publisher;
    std::wstring version;
    bool isUWP;
    HICON hIcon;
    std::wstring iconPath;
    
    ApplicationInfo(const std::wstring& appName, const std::wstring& appPath, 
                   const std::wstring& appDesc = L"", const std::wstring& appPublisher = L"", 
                   const std::wstring& appVersion = L"", bool uwp = false)
        : name(appName), path(appPath), description(appDesc), 
          publisher(appPublisher), version(appVersion), isUWP(uwp), hIcon(nullptr), iconPath(L"") {}
          
    ~ApplicationInfo() {
        if (hIcon) {
            DestroyIcon(hIcon);
        }
    }
    
    // Copy constructor
    ApplicationInfo(const ApplicationInfo& other) 
        : name(other.name), path(other.path), description(other.description),
          publisher(other.publisher), version(other.version), isUWP(other.isUWP),
          hIcon(nullptr), iconPath(other.iconPath) {
        if (other.hIcon) {
            hIcon = CopyIcon(other.hIcon);
        }
    }
    
    // Assignment operator
    ApplicationInfo& operator=(const ApplicationInfo& other) {
        if (this != &other) {
            if (hIcon) {
                DestroyIcon(hIcon);
                hIcon = nullptr;
            }
            
            name = other.name;
            path = other.path;
            description = other.description;
            publisher = other.publisher;
            version = other.version;
            isUWP = other.isUWP;
            iconPath = other.iconPath;
            
            if (other.hIcon) {
                hIcon = CopyIcon(other.hIcon);
            }
        }
        return *this;
    }
};

class ApplicationFinder {
public:
    ApplicationFinder();
    
    std::vector<ApplicationInfo> FindApplications(const std::wstring& searchTerm);
    void RefreshApplications();
    size_t GetApplicationCount() const { return m_applications.size(); }
    
private:
    std::vector<ApplicationInfo> m_applications;
    bool m_isInitialized;
    
    void InitializeApplications();
    void SearchInDirectory(const std::wstring& directory, bool recursive = false);
    void SearchInStartMenu();
    void SearchInProgramFiles();
    void SearchInCommonApplications();
    
    // Neue erweiterte Suchmethoden
    void SearchInRegistry();
    void SearchRegistryPath(HKEY hKey, const std::wstring& subKey);
    void SearchUWPApplications();
    void SearchWebBrowsers();
    void AddSystemTools();
    
    // Icon extraction methods
    HICON ExtractIconFromFile(const std::wstring& filePath);
    HICON ExtractIconFromShortcut(const std::wstring& lnkPath);
    HICON GetSystemIcon(const std::wstring& fileName);
    HICON GetDefaultApplicationIcon();
    void LoadIconForApplication(ApplicationInfo& app);
    
    // Hilfsmethoden
    bool IsExecutableFile(const std::wstring& filePath);
    std::wstring GetFileDescription(const std::wstring& filePath);
    std::wstring GetFileVersion(const std::wstring& filePath);
    std::wstring GetFilePublisher(const std::wstring& filePath);
    std::wstring ExtractApplicationName(const std::wstring& filePath);
    bool ContainsIgnoreCase(const std::wstring& text, const std::wstring& searchTerm);
    std::wstring GetRegistryString(HKEY hKey, const std::wstring& valueName);
    bool IsValidExecutablePath(const std::wstring& path);
}; 