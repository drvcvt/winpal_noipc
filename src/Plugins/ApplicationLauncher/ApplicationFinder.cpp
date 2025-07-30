#include "ApplicationFinder.h"
#include <windows.h>
#include <filesystem>
#include <algorithm>
#include <shlobj.h>
#include <set>
#include <cwctype>
#include <shellapi.h>
#include <comdef.h>
#include <wbemidl.h>
#include <thread>
#include <mutex>
#include <shlwapi.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")

ApplicationFinder::ApplicationFinder() : m_isInitialized(false) {
    InitializeApplications();
}

std::vector<ApplicationInfo> ApplicationFinder::FindApplications(const std::wstring& searchTerm) {
    if (!m_isInitialized) {
        InitializeApplications();
    }
    
    std::vector<ApplicationInfo> results;
    
    if (searchTerm.empty()) {
        return results;
    }
    
    std::wstring lowerSearchTerm = searchTerm;
    std::transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), ::towlower);
    
    // Simple fast search without complex caching
    for (const auto& app : m_applications) {
        std::wstring lowerName = app.name;
        std::wstring lowerDesc = app.description;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::towlower);
        std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::towlower);
        
        // Quick relevance check
        bool matches = false;
        if (lowerName.find(lowerSearchTerm) != std::wstring::npos ||
            lowerDesc.find(lowerSearchTerm) != std::wstring::npos) {
            matches = true;
        }
        
        if (matches) {
            results.push_back(app);
            // Limit results for performance
            if (results.size() >= 15) {
                break;
            }
        }
    }
    
    // Simple sorting by name length (shorter names are often more relevant)
    std::sort(results.begin(), results.end(), [&lowerSearchTerm](const ApplicationInfo& a, const ApplicationInfo& b) {
        std::wstring lowerA = a.name;
        std::wstring lowerB = b.name;
        std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::towlower);
        std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::towlower);
        
        // Exact matches first
        bool aExact = (lowerA == lowerSearchTerm);
        bool bExact = (lowerB == lowerSearchTerm);
        if (aExact && !bExact) return true;
        if (!aExact && bExact) return false;
        
        // Starts-with matches next
        bool aStarts = (lowerA.find(lowerSearchTerm) == 0);
        bool bStarts = (lowerB.find(lowerSearchTerm) == 0);
        if (aStarts && !bStarts) return true;
        if (!aStarts && bStarts) return false;
        
        // Then by name length
        if (a.name.length() != b.name.length()) {
            return a.name.length() < b.name.length();
        }
        
        return a.name < b.name;
    });
    
    return results;
}

void ApplicationFinder::RefreshApplications() {
    m_applications.clear();
    m_isInitialized = false;
    InitializeApplications();
}

void ApplicationFinder::InitializeApplications() {
    if (m_isInitialized) return;
    
    m_applications.clear();
    
    try {
        // Reihenfolge ist wichtig für Performance
        SearchInCommonApplications();
        AddSystemTools();
        SearchInStartMenu();
        SearchInRegistry();
        SearchWebBrowsers();
        SearchInProgramFiles();
        
        // UWP Apps in separatem Thread für bessere Performance
        std::thread uwpThread([this]() {
            try {
                SearchUWPApplications();
            } catch (...) {
                // Fehler bei UWP-Suche ignorieren
            }
        });
        uwpThread.detach();
        
        m_isInitialized = true;
    }
    catch (...) {
        // Bei Fehlern trotzdem als initialisiert markieren
        m_isInitialized = true;
    }
}

void ApplicationFinder::SearchInDirectory(const std::wstring& directory, bool recursive) {
    try {
        if (!std::filesystem::exists(directory)) {
            return;
        }
        
        std::set<std::wstring> processedFiles; // Duplikate vermeiden
        
        auto processFile = [&](const std::filesystem::directory_entry& entry) {
            if (entry.is_regular_file()) {
                std::wstring filePath = entry.path().wstring();
                
                // Duplikate vermeiden
                if (processedFiles.find(filePath) != processedFiles.end()) {
                    return;
                }
                processedFiles.insert(filePath);
                
                if (IsExecutableFile(filePath) && IsValidExecutablePath(filePath)) {
                    std::wstring name = ExtractApplicationName(filePath);
                    std::wstring description = GetFileDescription(filePath);
                    std::wstring publisher = GetFilePublisher(filePath);
                    std::wstring version = GetFileVersion(filePath);
                    
                    if (!name.empty()) {
                        ApplicationInfo app(name, filePath, description, publisher, version, false);
                        LoadIconForApplication(app);
                        m_applications.push_back(std::move(app));
                    }
                }
            }
        };
        
        if (recursive) {
            std::filesystem::recursive_directory_iterator iterator(directory, std::filesystem::directory_options::skip_permission_denied);
            for (const auto& entry : iterator) {
                processFile(entry);
            }
        } else {
            std::filesystem::directory_iterator iterator(directory, std::filesystem::directory_options::skip_permission_denied);
            for (const auto& entry : iterator) {
                processFile(entry);
            }
        }
    }
    catch (...) {
        // Fehler bei der Verzeichnissuche ignorieren
    }
}

void ApplicationFinder::SearchInStartMenu() {
    wchar_t startMenuPath[MAX_PATH];
    
    // Benutzer-Startmenü
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, 0, startMenuPath))) {
        SearchInDirectory(std::wstring(startMenuPath) + L"\\Programs", true);
    }
    
    // Gemeinsames Startmenü
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, startMenuPath))) {
        SearchInDirectory(std::wstring(startMenuPath) + L"\\Programs", true);
    }
}

void ApplicationFinder::SearchInProgramFiles() {
    wchar_t programFilesPath[MAX_PATH];
    
    // Program Files
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, programFilesPath))) {
        SearchInDirectory(programFilesPath, false); // Nur oberste Ebene für Performance
    }
    
    // Program Files (x86)
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, programFilesPath))) {
        SearchInDirectory(programFilesPath, false); // Nur oberste Ebene für Performance
    }
}

void ApplicationFinder::SearchInCommonApplications() {
    // Häufig verwendete System-Anwendungen mit besseren Beschreibungen
    std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> commonApps = {
        {L"Calculator", L"calc.exe", L"Windows Calculator"},
        {L"Notepad", L"notepad.exe", L"Text Editor"},
        {L"Task Manager", L"taskmgr.exe", L"System Process Manager"},
        {L"Command Prompt", L"cmd.exe", L"Command Line Interface"},
        {L"PowerShell", L"powershell.exe", L"Advanced Command Line"},
        {L"Registry Editor", L"regedit.exe", L"Windows Registry Editor"},
        {L"System Configuration", L"msconfig.exe", L"System Startup Configuration"},
        {L"Device Manager", L"devmgmt.msc", L"Hardware Device Manager"},
        {L"Event Viewer", L"eventvwr.msc", L"System Event Viewer"},
        {L"Services", L"services.msc", L"Windows Services Manager"},
        {L"Computer Management", L"compmgmt.msc", L"Computer Management Console"},
        {L"Disk Management", L"diskmgmt.msc", L"Disk and Volume Manager"},
        {L"System Information", L"msinfo32.exe", L"System Information Utility"},
        {L"Paint", L"mspaint.exe", L"Image Editor"},
        {L"Windows Explorer", L"explorer.exe", L"File Manager"},
        {L"Control Panel", L"control.exe", L"System Settings"},
        {L"Character Map", L"charmap.exe", L"Character and Symbol Viewer"},
        {L"System File Checker", L"sfc.exe", L"System File Integrity Check"},
        {L"Resource Monitor", L"resmon.exe", L"System Resource Monitor"},
        {L"Windows Update", L"ms-settings:windowsupdate", L"Windows Update Settings"}
    };
    
    for (const auto& app : commonApps) {
        ApplicationInfo appInfo(std::get<0>(app), std::get<1>(app), std::get<2>(app), L"Microsoft", L"", false);
        LoadIconForApplication(appInfo);
        m_applications.push_back(std::move(appInfo));
    }
}

void ApplicationFinder::SearchInRegistry() {
    try {
        // Uninstall-Register durchsuchen für installierte Programme
        SearchRegistryPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
        SearchRegistryPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
        SearchRegistryPath(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
        
        // App Paths durchsuchen
        SearchRegistryPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");
        SearchRegistryPath(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");
    }
    catch (...) {
        // Registry-Fehler ignorieren
    }
}

void ApplicationFinder::SearchRegistryPath(HKEY hKey, const std::wstring& subKey) {
    HKEY hSubKey;
    if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS) {
        return;
    }
    
    DWORD index = 0;
    wchar_t keyName[256];
    DWORD keyNameSize = sizeof(keyName) / sizeof(wchar_t);
    
    while (RegEnumKeyExW(hSubKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        HKEY hAppKey;
        if (RegOpenKeyExW(hSubKey, keyName, 0, KEY_READ, &hAppKey) == ERROR_SUCCESS) {
            std::wstring displayName = GetRegistryString(hAppKey, L"DisplayName");
            std::wstring installLocation = GetRegistryString(hAppKey, L"InstallLocation");
            std::wstring displayIcon = GetRegistryString(hAppKey, L"DisplayIcon");
            std::wstring publisher = GetRegistryString(hAppKey, L"Publisher");
            std::wstring version = GetRegistryString(hAppKey, L"DisplayVersion");
            
            if (!displayName.empty() && displayName.find(L"Update") == std::wstring::npos && 
                displayName.find(L"Hotfix") == std::wstring::npos &&
                displayName.find(L"Security") == std::wstring::npos) {
                
                std::wstring executablePath;
                
                // Versuche Executable-Pfad zu finden
                if (!installLocation.empty()) {
                    // Suche nach .exe-Dateien im Install-Verzeichnis
                    try {
                        for (const auto& entry : std::filesystem::directory_iterator(installLocation)) {
                            if (entry.is_regular_file() && entry.path().extension() == L".exe") {
                                executablePath = entry.path().wstring();
                                break;
                            }
                        }
                    } catch (...) {}
                }
                
                if (executablePath.empty() && !displayIcon.empty()) {
                    // Versuche Icon-Pfad als Executable zu verwenden
                    size_t commaPos = displayIcon.find(L',');
                    if (commaPos != std::wstring::npos) {
                        executablePath = displayIcon.substr(0, commaPos);
                    } else {
                        executablePath = displayIcon;
                    }
                }
                
                if (executablePath.empty()) {
                    executablePath = displayName; // Fallback
                }
                
                ApplicationInfo appInfo(displayName, executablePath, L"Installed Application", publisher, version, false);
                LoadIconForApplication(appInfo);
                m_applications.push_back(std::move(appInfo));
            }
            
            RegCloseKey(hAppKey);
        }
        
        index++;
        keyNameSize = sizeof(keyName) / sizeof(wchar_t);
    }
    
    RegCloseKey(hSubKey);
}

void ApplicationFinder::SearchUWPApplications() {
    // UWP Apps über PowerShell-Command abrufen
    // Dies ist komplexer und würde eine vollständige PowerShell-Integration benötigen
    // Für jetzt eine vereinfachte Implementation
    
    try {
        // Bekannte UWP-Apps hinzufügen
        std::vector<std::tuple<std::wstring, std::wstring, std::wstring, std::wstring>> uwpApps = {
            {L"Microsoft Store", L"ms-windows-store:", L"App Store", L"Microsoft"},
            {L"Settings", L"ms-settings:", L"Windows Settings", L"Microsoft"},
            {L"Mail", L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.mail", L"Email Client", L"Microsoft"},
            {L"Calendar", L"microsoft.windowscommunicationsapps_8wekyb3d8bbwe!microsoft.windowslive.calendar", L"Calendar App", L"Microsoft"},
            {L"Photos", L"Microsoft.Windows.Photos_8wekyb3d8bbwe!App", L"Photo Viewer and Editor", L"Microsoft"},
            {L"Calculator", L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App", L"Advanced Calculator", L"Microsoft"},
            {L"Microsoft Edge", L"microsoft-edge:", L"Web Browser", L"Microsoft"},
            {L"Xbox", L"Microsoft.XboxApp_8wekyb3d8bbwe!Microsoft.XboxApp", L"Xbox Gaming Platform", L"Microsoft"},
            {L"Spotify", L"SpotifyAB.SpotifyMusic_zpdnekdrzrea0!Spotify", L"Music Streaming", L"Spotify AB"},
            {L"Netflix", L"4DF9E0F8.Netflix_mcm4njqhnhss8!App", L"Video Streaming", L"Netflix"},
            {L"WhatsApp", L"5319275A.WhatsAppDesktop_cv1g1gvanyjgm!WhatsAppDesktop", L"Messaging App", L"WhatsApp"}
        };
        
        for (const auto& app : uwpApps) {
            ApplicationInfo appInfo(std::get<0>(app), std::get<1>(app), std::get<2>(app), std::get<3>(app), L"", true);
            LoadIconForApplication(appInfo);
            m_applications.push_back(std::move(appInfo));
        }
    }
    catch (...) {
        // UWP-Fehler ignorieren
    }
}

void ApplicationFinder::SearchWebBrowsers() {
    // Häufig verwendete Browser
    std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> browsers = {
        {L"Google Chrome", L"chrome.exe", L"Web Browser"},
        {L"Mozilla Firefox", L"firefox.exe", L"Web Browser"},
        {L"Microsoft Edge", L"msedge.exe", L"Web Browser"},
        {L"Opera", L"opera.exe", L"Web Browser"},
        {L"Brave", L"brave.exe", L"Privacy-focused Browser"},
        {L"Internet Explorer", L"iexplore.exe", L"Legacy Web Browser"}
    };
    
    for (const auto& browser : browsers) {
        ApplicationInfo appInfo(std::get<0>(browser), std::get<1>(browser), std::get<2>(browser), L"", L"", false);
        LoadIconForApplication(appInfo);
        m_applications.push_back(std::move(appInfo));
    }
}

void ApplicationFinder::AddSystemTools() {
    // Erweiterte System-Tools
    std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> tools = {
        {L"Windows Security", L"windowsdefender:", L"Antivirus and Security Settings"},
        {L"Disk Cleanup", L"cleanmgr.exe", L"Free up disk space"},
        {L"System Restore", L"rstrui.exe", L"Restore system to previous state"},
        {L"Windows Memory Diagnostic", L"mdsched.exe", L"Check computer memory"},
        {L"Performance Monitor", L"perfmon.exe", L"Monitor system performance"},
        {L"Group Policy Editor", L"gpedit.msc", L"Edit group policies"},
        {L"Local Security Policy", L"secpol.msc", L"Configure security policies"},
        {L"Certificate Manager", L"certmgr.msc", L"Manage certificates"},
        {L"Component Services", L"dcomcnfg.exe", L"Configure component services"},
        {L"Windows Firewall", L"wf.msc", L"Configure firewall settings"},
        {L"Task Scheduler", L"taskschd.msc", L"Schedule automated tasks"},
        {L"Network Connections", L"ncpa.cpl", L"Manage network connections"},
        {L"Sound Settings", L"mmsys.cpl", L"Configure audio devices"},
        {L"Display Settings", L"desk.cpl", L"Configure display properties"},
        {L"Power Options", L"powercfg.cpl", L"Configure power management"}
    };
    
    for (const auto& tool : tools) {
        ApplicationInfo appInfo(std::get<0>(tool), std::get<1>(tool), std::get<2>(tool), L"Microsoft", L"", false);
        LoadIconForApplication(appInfo);
        m_applications.push_back(std::move(appInfo));
    }
}

bool ApplicationFinder::IsExecutableFile(const std::wstring& filePath) {
    std::wstring extension = std::filesystem::path(filePath).extension().wstring();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);
    
    return extension == L".exe" || extension == L".lnk" || extension == L".msc" || 
           extension == L".cpl" || extension == L".scr";
}

std::wstring ApplicationFinder::GetFileDescription(const std::wstring& filePath) {
    try {
        DWORD handle = 0;
        DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &handle);
        if (size == 0) {
            return L"Application";
        }
        
        std::vector<BYTE> buffer(size);
        if (!GetFileVersionInfoW(filePath.c_str(), handle, size, buffer.data())) {
            return L"Application";
        }
        
        LPVOID lpBuffer = nullptr;
        UINT uLen = 0;
        
        if (VerQueryValueW(buffer.data(), L"\\StringFileInfo\\040904B0\\FileDescription", &lpBuffer, &uLen) && uLen > 0) {
            return std::wstring(static_cast<wchar_t*>(lpBuffer));
        }
        
        // Fallback für andere Sprachen
        if (VerQueryValueW(buffer.data(), L"\\StringFileInfo\\000004B0\\FileDescription", &lpBuffer, &uLen) && uLen > 0) {
            return std::wstring(static_cast<wchar_t*>(lpBuffer));
        }
    }
    catch (...) {}
    
    return L"Application";
}

std::wstring ApplicationFinder::GetFileVersion(const std::wstring& filePath) {
    try {
        DWORD handle = 0;
        DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &handle);
        if (size == 0) return L"";
        
        std::vector<BYTE> buffer(size);
        if (!GetFileVersionInfoW(filePath.c_str(), handle, size, buffer.data())) return L"";
        
        VS_FIXEDFILEINFO* pFileInfo = nullptr;
        UINT len = 0;
        
        if (VerQueryValueW(buffer.data(), L"\\", reinterpret_cast<LPVOID*>(&pFileInfo), &len)) {
            if (pFileInfo) {
                return std::to_wstring(HIWORD(pFileInfo->dwFileVersionMS)) + L"." +
                       std::to_wstring(LOWORD(pFileInfo->dwFileVersionMS)) + L"." +
                       std::to_wstring(HIWORD(pFileInfo->dwFileVersionLS)) + L"." +
                       std::to_wstring(LOWORD(pFileInfo->dwFileVersionLS));
            }
        }
    }
    catch (...) {}
    
    return L"";
}

std::wstring ApplicationFinder::GetFilePublisher(const std::wstring& filePath) {
    try {
        DWORD handle = 0;
        DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &handle);
        if (size == 0) return L"";
        
        std::vector<BYTE> buffer(size);
        if (!GetFileVersionInfoW(filePath.c_str(), handle, size, buffer.data())) return L"";
        
        LPVOID lpBuffer = nullptr;
        UINT uLen = 0;
        
        if (VerQueryValueW(buffer.data(), L"\\StringFileInfo\\040904B0\\CompanyName", &lpBuffer, &uLen) && uLen > 0) {
            return std::wstring(static_cast<wchar_t*>(lpBuffer));
        }
    }
    catch (...) {}
    
    return L"";
}

std::wstring ApplicationFinder::ExtractApplicationName(const std::wstring& filePath) {
    std::filesystem::path path(filePath);
    std::wstring fileName = path.stem().wstring();
    
    // Entferne häufige Suffixe und Präfixe
    std::vector<std::wstring> suffixesToRemove = {
        L"_setup", L"_install", L"_installer", L"Setup", L"Install", L"Installer",
        L"_x64", L"_x86", L"_win32", L"_win64", L"32bit", L"64bit"
    };
    
    for (const auto& suffix : suffixesToRemove) {
        size_t pos = fileName.find(suffix);
        if (pos != std::wstring::npos) {
            fileName = fileName.substr(0, pos);
            break;
        }
    }
    
    // Erster Buchstabe groß, Rest entsprechend formatiert
    if (!fileName.empty()) {
        fileName[0] = ::towupper(fileName[0]);
        for (size_t i = 1; i < fileName.length(); ++i) {
            if (fileName[i-1] == L' ' || fileName[i-1] == L'-' || fileName[i-1] == L'_') {
                fileName[i] = ::towupper(fileName[i]);
            }
        }
    }
    
    return fileName;
}

bool ApplicationFinder::ContainsIgnoreCase(const std::wstring& text, const std::wstring& searchTerm) {
    std::wstring lowerText = text;
    std::wstring lowerSearch = searchTerm;
    
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::towlower);
    std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::towlower);
    
    return lowerText.find(lowerSearch) != std::wstring::npos;
}

std::wstring ApplicationFinder::GetRegistryString(HKEY hKey, const std::wstring& valueName) {
    DWORD dataType;
    DWORD dataSize = 0;
    
    // Größe ermitteln
    LONG result = RegQueryValueExW(hKey, valueName.c_str(), NULL, &dataType, NULL, &dataSize);
    if (result != ERROR_SUCCESS || dataType != REG_SZ) {
        return L"";
    }
    
    // Daten lesen
    std::vector<wchar_t> buffer(dataSize / sizeof(wchar_t));
    result = RegQueryValueExW(hKey, valueName.c_str(), NULL, &dataType, 
                             reinterpret_cast<LPBYTE>(buffer.data()), &dataSize);
    
    if (result == ERROR_SUCCESS) {
        return std::wstring(buffer.data());
    }
    
    return L"";
}

bool ApplicationFinder::IsValidExecutablePath(const std::wstring& path) {
    // Filtere unerwünschte Executables aus
    std::wstring lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::towlower);
    
    // Vermeide Installer, Uninstaller, etc.
    std::vector<std::wstring> excludePatterns = {
        L"uninstall", L"uninst", L"setup", L"install", L"updater", L"update",
        L"launcher", L"helper", L"service", L"daemon", L"crash", L"error",
        L"temp", L"tmp", L"cache", L"backup", L"debug", L"test"
    };
    
    for (const auto& pattern : excludePatterns) {
        if (lowerPath.find(pattern) != std::wstring::npos) {
            return false;
        }
    }
    
    return true;
}

// Icon extraction methods implementation
HICON ApplicationFinder::ExtractIconFromFile(const std::wstring& filePath) {
    if (filePath.empty() || !std::filesystem::exists(filePath)) {
        return nullptr;
    }
    
    // Try to extract large icon first (32x32)
    HICON hIconLarge = nullptr;
    HICON hIconSmall = nullptr;
    
    UINT iconCount = ExtractIconExW(filePath.c_str(), -1, &hIconLarge, &hIconSmall, 1);
    if (iconCount > 0) {
        ExtractIconExW(filePath.c_str(), 0, &hIconLarge, &hIconSmall, 1);
        
        // Prefer large icon, fallback to small
        if (hIconLarge) {
            if (hIconSmall) DestroyIcon(hIconSmall);
            return hIconLarge;
        } else if (hIconSmall) {
            return hIconSmall;
        }
    }
    
    // Fallback: try using LoadImage
    HICON hIcon = static_cast<HICON>(LoadImageW(
        nullptr, 
        filePath.c_str(), 
        IMAGE_ICON, 
        32, 32, 
        LR_LOADFROMFILE | LR_DEFAULTSIZE
    ));
    
    return hIcon;
}

HICON ApplicationFinder::ExtractIconFromShortcut(const std::wstring& lnkPath) {
    if (lnkPath.empty() || !std::filesystem::exists(lnkPath)) {
        return nullptr;
    }
    
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) return nullptr;
    
    IShellLinkW* pShellLink = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, 
                         IID_IShellLinkW, reinterpret_cast<void**>(&pShellLink));
    
    if (FAILED(hr)) {
        CoUninitialize();
        return nullptr;
    }
    
    IPersistFile* pPersistFile = nullptr;
    hr = pShellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pPersistFile));
    
    if (FAILED(hr)) {
        pShellLink->Release();
        CoUninitialize();
        return nullptr;
    }
    
    hr = pPersistFile->Load(lnkPath.c_str(), STGM_READ);
    if (FAILED(hr)) {
        pPersistFile->Release();
        pShellLink->Release();
        CoUninitialize();
        return nullptr;
    }
    
    wchar_t targetPath[MAX_PATH];
    wchar_t iconPath[MAX_PATH];
    int iconIndex = 0;
    
    // Get target path
    hr = pShellLink->GetPath(targetPath, MAX_PATH, nullptr, SLGP_UNCPRIORITY);
    
    // Get icon information
    hr = pShellLink->GetIconLocation(iconPath, MAX_PATH, &iconIndex);
    
    pPersistFile->Release();
    pShellLink->Release();
    CoUninitialize();
    
    HICON hIcon = nullptr;
    
    // Try to extract from specified icon path first
    if (wcslen(iconPath) > 0 && std::filesystem::exists(iconPath)) {
        HICON hIconLarge = nullptr;
        HICON hIconSmall = nullptr;
        UINT count = ExtractIconExW(iconPath, iconIndex, &hIconLarge, &hIconSmall, 1);
        if (count > 0) {
            hIcon = hIconLarge ? hIconLarge : hIconSmall;
            if (hIconLarge && hIconSmall && hIconLarge != hIconSmall) {
                DestroyIcon(hIconSmall);
            }
        }
    }
    
    // Fallback to target executable
    if (!hIcon && wcslen(targetPath) > 0) {
        hIcon = ExtractIconFromFile(targetPath);
    }
    
    return hIcon;
}

HICON ApplicationFinder::GetSystemIcon(const std::wstring& fileName) {
    SHFILEINFOW sfi = {};
    DWORD_PTR result = SHGetFileInfoW(
        fileName.c_str(),
        FILE_ATTRIBUTE_NORMAL,
        &sfi,
        sizeof(sfi),
        SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES
    );
    
    return result ? sfi.hIcon : nullptr;
}

HICON ApplicationFinder::GetDefaultApplicationIcon() {
    // Get default application icon from shell32.dll
    HICON hIcon = nullptr;
    ExtractIconExW(L"shell32.dll", 2, &hIcon, nullptr, 1); // Generic application icon
    return hIcon;
}

void ApplicationFinder::LoadIconForApplication(ApplicationInfo& app) {
    HICON hIcon = nullptr;
    
    if (app.isUWP) {
        // For UWP apps, we can't easily extract icons, so use a default UWP icon
        hIcon = GetSystemIcon(L"shell32.dll");
        if (!hIcon) {
            ExtractIconExW(L"shell32.dll", 15, &hIcon, nullptr, 1); // UWP/Store app icon
        }
    } else {
        std::wstring extension = std::filesystem::path(app.path).extension().wstring();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);
        
        if (extension == L".lnk") {
            // Handle shortcuts
            hIcon = ExtractIconFromShortcut(app.path);
        } else if (extension == L".exe" || extension == L".com" || extension == L".scr") {
            // Handle executables
            hIcon = ExtractIconFromFile(app.path);
        } else if (extension == L".msc" || extension == L".cpl") {
            // Handle system management tools
            hIcon = GetSystemIcon(app.path);
        } else {
            // Handle other file types or system commands
            if (app.path.find(L"ms-settings:") != std::wstring::npos) {
                // Windows Settings
                ExtractIconExW(L"shell32.dll", 316, &hIcon, nullptr, 1);
            } else if (app.path.find(L"ms-windows-store:") != std::wstring::npos) {
                // Microsoft Store
                ExtractIconExW(L"shell32.dll", 15, &hIcon, nullptr, 1);
            } else {
                // Try to find executable in system paths
                std::wstring fullPath = app.path;
                if (!std::filesystem::exists(fullPath)) {
                    // Search in System32
                    std::wstring system32Path = L"C:\\Windows\\System32\\" + app.path;
                    if (std::filesystem::exists(system32Path)) {
                        fullPath = system32Path;
                    } else {
                        // Search in Windows directory
                        std::wstring windowsPath = L"C:\\Windows\\" + app.path;
                        if (std::filesystem::exists(windowsPath)) {
                            fullPath = windowsPath;
                        }
                    }
                }
                
                if (std::filesystem::exists(fullPath)) {
                    hIcon = ExtractIconFromFile(fullPath);
                } else {
                    // Use system icon for the file type
                    hIcon = GetSystemIcon(app.path);
                }
            }
        }
    }
    
    // Fallback to default application icon
    if (!hIcon) {
        hIcon = GetDefaultApplicationIcon();
    }
    
    app.hIcon = hIcon;
    if (hIcon) {
        app.iconPath = app.path; // Store original path for reference
    }
} 