#include "CommandManager.h"
#include <algorithm>
#include <cwctype>
#include <set>
#include <sstream>
#include "../Plugins/SystemSettings/SettingsCommand.h"
#include "../Plugins/FileTools/OpenFileExplorerCommand.h"
#include "../Plugins/FileTools/OpenDownloadsCommand.h"
#include "../Plugins/FileTools/OpenDesktopCommand.h"
#include "../Plugins/ApplicationLauncher/LaunchCalculatorCommand.h"
#include "../Plugins/ApplicationLauncher/LaunchNotepadCommand.h"
#include "../Plugins/ApplicationLauncher/LaunchTaskManagerCommand.h"
#include "../Plugins/ApplicationLauncher/GenericLaunchCommand.h"
#include "../Plugins/ApplicationLauncher/RefreshApplicationsCommand.h"
#include "../Plugins/SystemInfo/ShowSystemInfoCommand.h"
#include "../Plugins/SystemInfo/ShowDiskUsageCommand.h"
#include "../Plugins/NetworkTools/ShowNetworkInfoCommand.h"
#include "../Plugins/NetworkTools/PingCommand.h"
#include "../Plugins/ClipboardManager/ClearClipboardCommand.h"
#include "../Plugins/DeveloperTools/OpenGitBashCommand.h"
#include "../Plugins/DeveloperTools/OpenPowerShellCommand.h"
#include "../Plugins/ProcessTools/EnterProcessModeCommand.h"
#include "../Plugins/ProcessTools/OpenProcessPathCommand.h"
#include "../Plugins/ProcessTools/TerminateProcessCommand.h"
#include <vector>
#include <utility>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <sstream>

void CommandManager::RegisterCommand(std::unique_ptr<ICommand> command)
{
    m_commands.push_back(std::move(command));
}

void CommandManager::RegisterFileToolsCommands()
{
    RegisterCommand(std::make_unique<OpenFileExplorerCommand>());
    RegisterCommand(std::make_unique<OpenDownloadsCommand>());
    RegisterCommand(std::make_unique<OpenDesktopCommand>());
}

void CommandManager::RegisterApplicationLauncherCommands()
{
    // Generischer Launch-Befehl (neue Funktionalität)
    RegisterCommand(std::make_unique<GenericLaunchCommand>());
    RegisterCommand(std::make_unique<RefreshApplicationsCommand>());

    // Bestehende spezifische Launch-Befehle (für Kompatibilität beibehalten)
    RegisterCommand(std::make_unique<LaunchCalculatorCommand>());
    RegisterCommand(std::make_unique<LaunchNotepadCommand>());
    RegisterCommand(std::make_unique<LaunchTaskManagerCommand>());
}

void CommandManager::RegisterSystemInfoCommands()
{
    RegisterCommand(std::make_unique<ShowSystemInfoCommand>());
    RegisterCommand(std::make_unique<ShowDiskUsageCommand>());
}

void CommandManager::RegisterNetworkToolsCommands()
{
    RegisterCommand(std::make_unique<ShowNetworkInfoCommand>());
    RegisterCommand(std::make_unique<PingCommand>());
}

void CommandManager::RegisterClipboardManagerCommands()
{
    RegisterCommand(std::make_unique<ClearClipboardCommand>());
}

void CommandManager::RegisterDeveloperToolsCommands()
{
    RegisterCommand(std::make_unique<OpenGitBashCommand>());
    RegisterCommand(std::make_unique<OpenPowerShellCommand>());
}

void CommandManager::RegisterProcessToolsCommands()
{
    RegisterCommand(std::make_unique<EnterProcessModeCommand>());
    RegisterCommand(std::make_unique<OpenProcessPathCommand>());
    RegisterCommand(std::make_unique<TerminateProcessCommand>());
}

void CommandManager::RegisterAllPlugins()
{
    RegisterSettingsCommands();
    RegisterProcessToolsCommands();
    RegisterFileToolsCommands();
    RegisterApplicationLauncherCommands();
    RegisterSystemInfoCommands();
    RegisterNetworkToolsCommands();
    RegisterClipboardManagerCommands();
    RegisterDeveloperToolsCommands();
}

void CommandManager::RegisterSettingsCommands()
{
    std::vector<std::pair<std::wstring, std::wstring>> settings;
    settings.emplace_back(L"Display", L"ms-settings:display");
    settings.emplace_back(L"Night light", L"ms-settings:nightlight");
    settings.emplace_back(L"Advanced scaling settings", L"ms-settings:display-advanced");
    settings.emplace_back(L"Connect to a wireless display", L"ms-settings-connectabledevices:devicediscovery");
    settings.emplace_back(L"Graphics settings", L"ms-settings:display-advancedgraphics");
    settings.emplace_back(L"Display orientation", L"ms-settings:screenrotation");
    settings.emplace_back(L"Sound", L"ms-settings:sound");
    settings.emplace_back(L"Manage sound devices", L"ms-settings:sound-devices");
    settings.emplace_back(L"App volume and device preferences", L"ms-settings:apps-volume");
    settings.emplace_back(L"Notifications & actions", L"ms-settings:notifications");
    settings.emplace_back(L"Focus assist", L"ms-settings:quiethours");
    settings.emplace_back(L"Power & sleep", L"ms-settings:powersleep");
    settings.emplace_back(L"Battery", L"ms-settings:batterysaver");
    settings.emplace_back(L"Storage", L"ms-settings:storagesense");
    settings.emplace_back(L"Tablet mode", L"ms-settings:tabletmode");
    settings.emplace_back(L"Multitasking", L"ms-settings:multitasking");
    settings.emplace_back(L"Projecting to this PC", L"ms-settings:project");
    settings.emplace_back(L"Shared experiences", L"ms-settings:crossdevice");
    settings.emplace_back(L"Clipboard", L"ms-settings:clipboard");
    settings.emplace_back(L"Remote Desktop", L"ms-settings:remotedesktop");
    settings.emplace_back(L"About", L"ms-settings:about");
    settings.emplace_back(L"Bluetooth & other devices", L"ms-settings:bluetooth");
    settings.emplace_back(L"Printers & scanners", L"ms-settings:printers");
    settings.emplace_back(L"Mouse", L"ms-settings:mousetouchpad");
    settings.emplace_back(L"Touchpad", L"ms-settings:devices-touchpad");
    settings.emplace_back(L"Typing", L"ms-settings:typing");
    settings.emplace_back(L"Pen & Windows Ink", L"ms-settings:pen");
    settings.emplace_back(L"AutoPlay", L"ms-settings:autoplay");
    settings.emplace_back(L"USB", L"ms-settings:usb");
    settings.emplace_back(L"Network & Internet", L"ms-settings:network-status");
    settings.emplace_back(L"Wi-Fi", L"ms-settings:network-wifi");
    settings.emplace_back(L"Ethernet", L"ms-settings:network-ethernet");
    settings.emplace_back(L"VPN", L"ms-settings:network-vpn");
    settings.emplace_back(L"Airplane mode", L"ms-settings:network-airplanemode");
    settings.emplace_back(L"Mobile hotspot", L"ms-settings:network-mobilehotspot");
    settings.emplace_back(L"Proxy", L"ms-settings:network-proxy");
    settings.emplace_back(L"Personalization", L"ms-settings:personalization");
    settings.emplace_back(L"Background", L"ms-settings:personalization-background");
    settings.emplace_back(L"Colors", L"ms-settings:colors");
    settings.emplace_back(L"Lock screen", L"ms-settings:lockscreen");
    settings.emplace_back(L"Themes", L"ms-settings:themes");
    settings.emplace_back(L"Fonts", L"ms-settings:fonts");
    settings.emplace_back(L"Start", L"ms-settings:personalization-start");
    settings.emplace_back(L"Taskbar", L"ms-settings:taskbar");
    settings.emplace_back(L"Apps & features", L"ms-settings:appsfeatures");
    settings.emplace_back(L"Default apps", L"ms-settings:defaultapps");
    settings.emplace_back(L"Offline maps", L"ms-settings:maps");
    settings.emplace_back(L"Apps for websites", L"ms-settings:appsforwebsites");
    settings.emplace_back(L"Video playback", L"ms-settings:videoplayback");
    settings.emplace_back(L"Startup", L"ms-settings:startupapps");
    settings.emplace_back(L"Your info", L"ms-settings:yourinfo");
    settings.emplace_back(L"Email & accounts", L"ms-settings:emailandaccounts");
    settings.emplace_back(L"Sign-in options", L"ms-settings:signinoptions");
    settings.emplace_back(L"Access work or school", L"ms-settings:workplace");
    settings.emplace_back(L"Family & other people", L"ms-settings:otherusers");
    settings.emplace_back(L"Sync your settings", L"ms-settings:sync");
    settings.emplace_back(L"Date & time", L"ms-settings:dateandtime");
    settings.emplace_back(L"Region", L"ms-settings:regionformatting");
    settings.emplace_back(L"Language", L"ms-settings:regionlanguage");
    settings.emplace_back(L"Speech", L"ms-settings:speech");
    settings.emplace_back(L"Game bar", L"ms-settings:gaming-gamebar");
    settings.emplace_back(L"Captures", L"ms-settings:gaming-gamedvr");
    settings.emplace_back(L"Game Mode", L"ms-settings:gaming-gamemode");
    settings.emplace_back(L"Xbox Networking", L"ms-settings:gaming-xboxnetworking");
    settings.emplace_back(L"Display (Ease of Access)", L"ms-settings:easeofaccess-display");
    settings.emplace_back(L"Mouse Pointer", L"ms-settings:easeofaccess-cursorandpointersize");
    settings.emplace_back(L"Text Cursor", L"ms-settings:easeofaccess-cursor");
    settings.emplace_back(L"Magnifier", L"ms-settings:easeofaccess-magnifier");
    settings.emplace_back(L"Color Filters", L"ms-settings:easeofaccess-colorfilter");
    settings.emplace_back(L"High Contrast", L"ms-settings:easeofaccess-highcontrast");
    settings.emplace_back(L"Narrator", L"ms-settings:easeofaccess-narrator");
    settings.emplace_back(L"Audio (Ease of Access)", L"ms-settings:easeofaccess-audio");
    settings.emplace_back(L"Closed captions", L"ms-settings:easeofaccess-closedcaptioning");
    settings.emplace_back(L"Speech (Ease of Access)", L"ms-settings:easeofaccess-speechrecognition");
    settings.emplace_back(L"Keyboard (Ease of Access)", L"ms-settings:easeofaccess-keyboard");
    settings.emplace_back(L"Mouse (Ease of Access)", L"ms-settings:easeofaccess-mouse");
    settings.emplace_back(L"Eye Control", L"ms-settings:easeofaccess-eyecontrol");
    settings.emplace_back(L"Privacy", L"ms-settings:privacy");
    settings.emplace_back(L"Location", L"ms-settings:privacy-location");
    settings.emplace_back(L"Camera", L"ms-settings:privacy-webcam");
    settings.emplace_back(L"Microphone", L"ms-settings:privacy-microphone");
    settings.emplace_back(L"Voice activation", L"ms-settings:privacy-voiceactivation");
    settings.emplace_back(L"Notifications (Privacy)", L"ms-settings:privacy-notifications");
    settings.emplace_back(L"Account info", L"ms-settings:privacy-accountinfo");
    settings.emplace_back(L"Contacts", L"ms-settings:privacy-contacts");
    settings.emplace_back(L"Calendar", L"ms-settings:privacy-calendar");
    settings.emplace_back(L"Phone calls", L"ms-settings:privacy-phonecalls");
    settings.emplace_back(L"Call history", L"ms-settings:privacy-callhistory");
    settings.emplace_back(L"Email (Privacy)", L"ms-settings:privacy-email");
    settings.emplace_back(L"Tasks", L"ms-settings:privacy-tasks");
    settings.emplace_back(L"Messaging", L"ms-settings:privacy-messaging");
    settings.emplace_back(L"Radios", L"ms-settings:privacy-radios");
    settings.emplace_back(L"Other devices", L"ms-settings:privacy-customdevices");
    settings.emplace_back(L"Background apps", L"ms-settings:privacy-backgroundapps");
    settings.emplace_back(L"App diagnostics", L"ms-settings:privacy-appdiagnostics");
    settings.emplace_back(L"Automatic file downloads", L"ms-settings:privacy-automaticfiledownloads");
    settings.emplace_back(L"Documents", L"ms-settings:privacy-documents");
    settings.emplace_back(L"Pictures", L"ms-settings:privacy-pictures");
    settings.emplace_back(L"Videos", L"ms-settings:privacy-videos");
    settings.emplace_back(L"File system", L"ms-settings:privacy-broadfilesystemaccess");
    settings.emplace_back(L"Windows Update", L"ms-settings:windowsupdate");
    settings.emplace_back(L"Update history", L"ms-settings:windowsupdate-history");
    settings.emplace_back(L"Advanced options", L"ms-settings:windowsupdate-options");
    settings.emplace_back(L"Delivery Optimization", L"ms-settings:delivery-optimization");
    settings.emplace_back(L"Windows Security", L"ms-settings:windowsdefender");
    settings.emplace_back(L"Backup", L"ms-settings:backup");
    settings.emplace_back(L"Troubleshoot", L"ms-settings:troubleshoot");
    settings.emplace_back(L"Recovery", L"ms-settings:recovery");
    settings.emplace_back(L"Activation", L"ms-settings:activation");
    settings.emplace_back(L"Find My Device", L"ms-settings:findmydevice");
    settings.emplace_back(L"For developers", L"ms-settings:developers");
    settings.emplace_back(L"Windows Insider Program", L"ms-settings:windowsinsider");

    for (const auto& setting : settings)
    {
        m_commands.push_back(std::make_unique<SettingsCommand>(setting.first, setting.second));
    }
}

std::vector<ICommand*> CommandManager::FindCommands(const std::wstring& query)
{
    auto results = FindCommandsWithRelevance(query);
    std::vector<ICommand*> found_commands;
    
    for (const auto& result : results) {
        found_commands.push_back(result.command);
    }
    
    return found_commands;
}

std::vector<SearchResult> CommandManager::FindCommandsWithRelevance(const std::wstring& query)
{
    std::vector<SearchResult> results;
    
    if (query.empty()) {
        return results;
    }
    
    std::wstring lowerQuery = ToLower(query);
    auto queryWords = SplitQuery(lowerQuery);
    
    for (const auto& command : m_commands)
    {
        std::wstring commandName = command->GetName();
        std::wstring commandDesc = command->GetDescription();
        
        SearchResult::MatchType matchType;
        std::wstring matchedText;
        double relevanceScore = CalculateRelevanceScore(commandName, commandDesc, lowerQuery, matchType, matchedText);
        
        if (relevanceScore > 0.0) {
            // Frequency boost based on execution history
            double frequencyBoost = CalculateFrequencyBoost(command.get());
            relevanceScore = relevanceScore * (1.0 + frequencyBoost);
            
            results.emplace_back(command.get(), relevanceScore, matchedText, matchType);
        }
    }
    
    // Sort by relevance score (highest first)
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.relevanceScore > b.relevanceScore;
    });
    
    // Limit results for performance (reduziert für bessere Responsiveness)
    if (results.size() > 8) {
        results.erase(results.begin() + 8, results.end());
    }
    
    return results;
}

std::vector<std::wstring> CommandManager::GetSearchSuggestions(const std::wstring& partialQuery, int maxSuggestions)
{
    std::vector<std::wstring> suggestions;
    
    if (partialQuery.empty()) {
        return suggestions;
    }
    
    std::wstring lowerQuery = ToLower(partialQuery);
    std::set<std::wstring> uniqueSuggestions;
    
    for (const auto& command : m_commands) {
        std::wstring lowerName = ToLower(command->GetName());
        
        // Exact matches or starts with
        if (lowerName.find(lowerQuery) == 0) {
            uniqueSuggestions.insert(command->GetName());
        }
        // Contains matches
        else if (lowerName.find(lowerQuery) != std::wstring::npos) {
            uniqueSuggestions.insert(command->GetName());
        }
        
        // Add description matches
        std::wstring lowerDesc = ToLower(command->GetDescription());
        if (lowerDesc.find(lowerQuery) != std::wstring::npos) {
            uniqueSuggestions.insert(command->GetName());
        }
    }
    
    // Convert to vector and limit results
    for (const auto& suggestion : uniqueSuggestions) {
        if (suggestions.size() >= maxSuggestions) break;
        suggestions.push_back(suggestion);
    }
    
    return suggestions;
}

double CommandManager::CalculateRelevanceScore(const std::wstring& commandName, const std::wstring& commandDesc, 
                                              const std::wstring& query, SearchResult::MatchType& matchType, 
                                              std::wstring& matchedText)
{
    std::wstring lowerName = ToLower(commandName);
    std::wstring lowerDesc = ToLower(commandDesc);
    std::wstring lowerQuery = ToLower(query);
    
    // Exact name match - highest priority
    if (lowerName == lowerQuery) {
        matchType = SearchResult::EXACT_NAME;
        matchedText = commandName;
        return 100.0;
    }
    
    // Starts with name match
    if (lowerName.find(lowerQuery) == 0) {
        matchType = SearchResult::STARTS_WITH_NAME;
        matchedText = commandName;
        return 90.0;
    }
    
    // Contains in name
    if (lowerName.find(lowerQuery) != std::wstring::npos) {
        matchType = SearchResult::CONTAINS_NAME;
        matchedText = commandName;
        return 80.0;
    }
    
    // Exact description match
    if (lowerDesc == lowerQuery) {
        matchType = SearchResult::EXACT_DESCRIPTION;
        matchedText = commandDesc;
        return 70.0;
    }
    
    // Contains in description
    if (lowerDesc.find(lowerQuery) != std::wstring::npos) {
        matchType = SearchResult::CONTAINS_DESCRIPTION;
        matchedText = commandDesc;
        return 60.0;
    }
    
    // Fuzzy matching on name
    double fuzzyNameScore = CalculateFuzzyScore(lowerName, lowerQuery);
    if (fuzzyNameScore > 0.6) {
        matchType = SearchResult::FUZZY_NAME;
        matchedText = commandName;
        return 50.0 * fuzzyNameScore;
    }
    
    // Fuzzy matching on description
    double fuzzyDescScore = CalculateFuzzyScore(lowerDesc, lowerQuery);
    if (fuzzyDescScore > 0.5) {
        matchType = SearchResult::FUZZY_DESCRIPTION;
        matchedText = commandDesc;
        return 40.0 * fuzzyDescScore;
    }
    
    return 0.0; // No match
}

double CommandManager::CalculateFuzzyScore(const std::wstring& text, const std::wstring& query)
{
    if (query.empty() || text.empty()) return 0.0;
    
    // Simple Levenshtein distance-based fuzzy matching
    size_t textLen = text.length();
    size_t queryLen = query.length();
    
    // If lengths are very different, reduce score
    size_t minLen = (textLen < queryLen) ? textLen : queryLen;
    size_t maxLen = (textLen > queryLen) ? textLen : queryLen;
    double lengthRatio = static_cast<double>(minLen) / maxLen;
    if (lengthRatio < 0.5) return 0.0;
    
    // Calculate how many characters match in sequence
    size_t matches = 0;
    size_t textPos = 0;
    
    for (size_t i = 0; i < queryLen && textPos < textLen; ++i) {
        while (textPos < textLen && text[textPos] != query[i]) {
            textPos++;
        }
        if (textPos < textLen) {
            matches++;
            textPos++;
        }
    }
    
    double matchRatio = static_cast<double>(matches) / queryLen;
    return matchRatio * lengthRatio;
}

double CommandManager::CalculateFrequencyBoost(ICommand* command)
{
    const auto& history = m_executionHistory.GetHistory();
    int executionCount = 0;
    
    for (const auto& entry : history) {
        if (entry.commandName == command->GetName()) {
            executionCount++;
        }
    }
    
    // Return boost factor (0.0 to 0.5 for 50% max boost)
    double boost = executionCount * 0.1;
    return (boost < 0.5) ? boost : 0.5;
}

std::vector<std::wstring> CommandManager::SplitQuery(const std::wstring& query)
{
    std::vector<std::wstring> words;
    std::wistringstream iss(query);
    std::wstring word;
    
    while (iss >> word) {
        words.push_back(word);
    }
    
    return words;
}

bool CommandManager::ContainsIgnoreCase(const std::wstring& text, const std::wstring& search)
{
    std::wstring lowerText = ToLower(text);
    std::wstring lowerSearch = ToLower(search);
    return lowerText.find(lowerSearch) != std::wstring::npos;
}

std::wstring CommandManager::ToLower(const std::wstring& text)
{
    std::wstring result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

std::vector<ICommand*> CommandManager::GetCommandsByCategory(CommandCategory category)
{
    std::vector<ICommand*> category_commands;
    
    for (const auto& command : m_commands)
    {
        if (command->GetCategory() == category)
        {
            category_commands.push_back(command.get());
        }
    }
    
    return category_commands;
}

void CommandManager::ExecuteCommand(ICommand* command) {
    if (command != nullptr) {
        // Zum Verlauf hinzufügen
        m_executionHistory.AddExecution(command);
        
        // Command ausführen
        command->Execute();
    }
}

void CommandManager::ExecutePowerShellCommand(const std::wstring& command) {
    // Zum Verlauf hinzufügen
    m_executionHistory.AddPowerShellExecution(command);
}

ExecutionHistory& CommandManager::GetExecutionHistory() {
    return m_executionHistory;
}

const ExecutionHistory& CommandManager::GetExecutionHistory() const {
    return m_executionHistory;
}

// Neue Shebang-Command-Implementierung
bool CommandManager::IsShebangCommand(const std::wstring& input) const {
    return !input.empty() && input[0] == L'!';
}

bool CommandManager::ExecuteShebangCommand(const std::wstring& input) {
    if (!IsShebangCommand(input) || input.length() < 2) {
        return false;
    }
    
    wchar_t commandType = ::towlower(input[1]);
    
    // Extrahiere den Rest als Argument (skip "!x ")
    std::wstring argument;
    if (input.length() > 3 && input[2] == L' ') {
        argument = input.substr(3);
        
        // Entferne führende und nachfolgende Leerzeichen
        size_t start = argument.find_first_not_of(L' ');
        size_t end = argument.find_last_not_of(L' ');
        if (start != std::wstring::npos && end != std::wstring::npos) {
            argument = argument.substr(start, end - start + 1);
        }
    }
    
    switch (commandType) {
        case L'l': // Launch command
            return ExecuteLaunchCommand(argument);
            
        case L't': // Terminate command
            return ExecuteTerminateCommand(argument);
            
        case L'f': // File Tools command
            return ExecuteFileToolCommand(argument);
            
        case L's': // System Info command
            return ExecuteSystemInfoCommand(argument);
            
        case L'n': // Network Tools command
            return ExecuteNetworkCommand(argument);
            
        case L'd': // Developer Tools command
            return ExecuteDeveloperCommand(argument);
            
        case L'c': // Clipboard Manager command
            return ExecuteClipboardCommand(argument);
            
        case L'z': // Settings command
            return ExecuteSettingsCommand(argument);
            
        default:
            return false;
    }
}

bool CommandManager::ExecuteLaunchCommand(const std::wstring& appName) {
    if (appName.empty()) {
        return false;
    }
    
    // Führe den Befehl direkt aus. Dies ist der robusteste Weg für Hotkeys.
    // Wir umgehen hier bewusst die komplexere anwendungsinterne Suche,
    // die für die interaktive UI gedacht ist.
    HINSTANCE result = ShellExecuteW(NULL, L"open", appName.c_str(), NULL, NULL, SW_SHOWNORMAL);
    bool success = (reinterpret_cast<INT_PTR>(result) > 32);
    
    if (success) {
        m_executionHistory.AddExecution(
            L"Launch " + appName,
            L"Direct application launch via hotkey",
            CommandCategory::APPLICATION_LAUNCHER
        );
    }
    
    return success;
}

bool CommandManager::ExecuteTerminateCommand(const std::wstring& processName) {
    if (processName.empty()) {
        MessageBoxW(NULL, L"Kein Prozessname angegeben.", L"WinPal - Fehler", MB_ICONERROR | MB_OK);
        return false;
    }
    
    // Prozess-Snapshot erstellen
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        MessageBoxW(NULL, L"Fehler beim Erstellen des Prozess-Snapshots.", L"WinPal - Fehler", MB_ICONERROR | MB_OK);
        return false;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    bool found = false;
    std::vector<DWORD> processIds;
    std::vector<std::wstring> foundProcessNames;
    
    // Durchsuche alle Prozesse
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            std::wstring exeFile = pe32.szExeFile;
            std::wstring lowerExeFile = exeFile;
            std::wstring lowerProcessName = processName;
            
            // Konvertiere zu Kleinbuchstaben für Vergleich
            std::transform(lowerExeFile.begin(), lowerExeFile.end(), lowerExeFile.begin(), ::towlower);
            std::transform(lowerProcessName.begin(), lowerProcessName.end(), lowerProcessName.begin(), ::towlower);
            
            // Überprüfe ob der Prozessname übereinstimmt (exakt oder enthält)
            if (lowerExeFile == lowerProcessName || 
                lowerExeFile == lowerProcessName + L".exe" ||
                lowerExeFile.find(lowerProcessName) != std::wstring::npos) {
                processIds.push_back(pe32.th32ProcessID);
                foundProcessNames.push_back(exeFile);
                found = true;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    
    if (!found) {
        std::wstring errorMsg = L"Prozess '" + processName + L"' nicht gefunden.";
        MessageBoxW(NULL, errorMsg.c_str(), L"WinPal - Prozess nicht gefunden", MB_ICONWARNING | MB_OK);
        return false;
    }
    
    // Terminiere gefundene Prozesse
    int terminatedCount = 0;
    int failedCount = 0;
    std::wstring failedProcesses;
    
    for (size_t i = 0; i < processIds.size(); ++i) {
        DWORD pid = processIds[i];
        std::wstring processNameForMsg = foundProcessNames[i];
        
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess != NULL) {
            if (TerminateProcess(hProcess, 1)) {
                terminatedCount++;
            } else {
                failedCount++;
                if (!failedProcesses.empty()) failedProcesses += L", ";
                failedProcesses += processNameForMsg + L" (PID: " + std::to_wstring(pid) + L")";
            }
            CloseHandle(hProcess);
        } else {
            failedCount++;
            if (!failedProcesses.empty()) failedProcesses += L", ";
            failedProcesses += processNameForMsg + L" (PID: " + std::to_wstring(pid) + L") - Zugriff verweigert";
        }
    }
    
    // Ergebnis-Benachrichtigungen
    if (terminatedCount > 0) {
        // Füge zum Verlauf hinzu
        std::wstring description = L"Terminated " + std::to_wstring(terminatedCount) + L" process(es)";
        if (failedCount > 0) {
            description += L", " + std::to_wstring(failedCount) + L" failed";
        }
        m_executionHistory.AddExecution(
            L"Terminate " + processName,
            description,
            CommandCategory::PROCESS_TOOLS
        );
        
        // Erfolgs-Benachrichtigung
        std::wstring successMsg = std::to_wstring(terminatedCount) + L" Prozess(e) erfolgreich beendet.";
        if (failedCount > 0) {
            successMsg += L"\n\nFehlgeschlagen: " + std::to_wstring(failedCount) + L" Prozess(e):\n" + failedProcesses;
            MessageBoxW(NULL, successMsg.c_str(), L"WinPal - Prozess teilweise beendet", MB_ICONWARNING | MB_OK);
        } else {
            MessageBoxW(NULL, successMsg.c_str(), L"WinPal - Prozess beendet", MB_ICONINFORMATION | MB_OK);
        }
        return true;
    } else {
        // Alle Terminierungen fehlgeschlagen
        std::wstring errorMsg = L"Alle " + std::to_wstring(failedCount) + L" gefundenen Prozesse konnten nicht beendet werden:\n" + failedProcesses;
        errorMsg += L"\n\nMögliche Ursachen:\n• Unzureichende Berechtigungen\n• Systemgeschützte Prozesse\n• Prozess bereits beendet";
        MessageBoxW(NULL, errorMsg.c_str(), L"WinPal - Terminierung fehlgeschlagen", MB_ICONERROR | MB_OK);
        return false;
    }
}

// Natürliche Command-Implementierung
bool CommandManager::IsNaturalCommand(const std::wstring& input) const {
    if (input.empty()) {
        return false;
    }
    
    // Konvertiere zu Kleinbuchstaben für Vergleich
    std::wstring lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::towlower);
    
    // Prüfe auf bekannte Command-Wörter am Anfang
    return (lowerInput.find(L"launch ") == 0 || 
            lowerInput.find(L"start ") == 0 ||
            lowerInput.find(L"run ") == 0 ||
            lowerInput.find(L"open ") == 0 ||
            lowerInput.find(L"terminate ") == 0 ||
            lowerInput.find(L"term ") == 0 ||
            lowerInput.find(L"kill ") == 0 ||
            lowerInput.find(L"stop ") == 0);
}

bool CommandManager::ExecuteNaturalCommand(const std::wstring& input) {
    auto [commandWord, argument] = ParseNaturalCommand(input);
    
    if (commandWord.empty() || argument.empty()) {
        return false;
    }
    
    // Konvertiere Command-Wort zu Kleinbuchstaben
    std::wstring lowerCommand = commandWord;
    std::transform(lowerCommand.begin(), lowerCommand.end(), lowerCommand.begin(), ::towlower);
    
    // Launch/Start/Run/Open Commands
    if (lowerCommand == L"launch" || lowerCommand == L"start" || 
        lowerCommand == L"run" || lowerCommand == L"open") {
        return ExecuteLaunchCommand(argument);
    }
    
    // Terminate/Term/Kill/Stop Commands
    if (lowerCommand == L"terminate" || lowerCommand == L"term" ||
        lowerCommand == L"kill" || lowerCommand == L"stop") {
        return ExecuteTerminateCommand(argument);
    }
    
    return false;
}

std::pair<std::wstring, std::wstring> CommandManager::ParseNaturalCommand(const std::wstring& input) const {
    if (input.empty()) {
        return {L"", L""};
    }
    
    // Finde das erste Leerzeichen
    size_t spacePos = input.find(L' ');
    if (spacePos == std::wstring::npos) {
        return {L"", L""};
    }
    
    std::wstring commandWord = input.substr(0, spacePos);
    std::wstring argument = input.substr(spacePos + 1);
    
    // Entferne führende und nachfolgende Leerzeichen vom Argument
    size_t start = argument.find_first_not_of(L' ');
    size_t end = argument.find_last_not_of(L' ');
    if (start != std::wstring::npos && end != std::wstring::npos) {
        argument = argument.substr(start, end - start + 1);
    } else {
        argument = L"";
    }
    
    return {commandWord, argument};
}

// Neue Shebang-Command-Implementierungen

bool CommandManager::ExecuteFileToolCommand(const std::wstring& target) {
    std::wstring lowerTarget = target;
    std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::towlower);
    
    if (lowerTarget.empty() || lowerTarget == L"explorer" || lowerTarget == L"fileexplorer") {
        // Open File Explorer
        ShellExecuteW(NULL, L"open", L"explorer.exe", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Open File Explorer", L"Windows File Explorer opened", CommandCategory::FILE_TOOLS);
        return true;
    }
    else if (lowerTarget == L"downloads" || lowerTarget == L"download") {
        // Open Downloads
        ShellExecuteW(NULL, L"open", L"shell:Downloads", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Open Downloads Folder", L"Downloads folder opened", CommandCategory::FILE_TOOLS);
        return true;
    }
    else if (lowerTarget == L"desktop") {
        // Open Desktop
        ShellExecuteW(NULL, L"open", L"shell:Desktop", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Open Desktop Folder", L"Desktop folder opened", CommandCategory::FILE_TOOLS);
        return true;
    }
    
    return false;
}

bool CommandManager::ExecuteSystemInfoCommand(const std::wstring& infoType) {
    std::wstring lowerType = infoType;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::towlower);
    
    if (lowerType.empty() || lowerType == L"info" || lowerType == L"system") {
        // Show System Information
        ShellExecuteW(NULL, L"open", L"msinfo32.exe", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Show System Information", L"System Information utility opened", CommandCategory::SYSTEM_INFO);
        return true;
    }
    else if (lowerType == L"disk" || lowerType == L"diskusage" || lowerType == L"cleanup") {
        // Show Disk Usage
        ShellExecuteW(NULL, L"open", L"cleanmgr.exe", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Show Disk Usage", L"Disk Cleanup utility opened", CommandCategory::SYSTEM_INFO);
        return true;
    }
    
    return false;
}

bool CommandManager::ExecuteNetworkCommand(const std::wstring& networkTool) {
    std::wstring lowerTool = networkTool;
    std::transform(lowerTool.begin(), lowerTool.end(), lowerTool.begin(), ::towlower);
    
    if (lowerTool.empty() || lowerTool == L"ping") {
        // Ping Google DNS
        ShellExecuteW(NULL, L"open", L"cmd.exe", L"/k ping 8.8.8.8", NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Ping Google DNS", L"Network connectivity test to 8.8.8.8", CommandCategory::NETWORK_TOOLS);
        return true;
    }
    else if (lowerTool == L"info" || lowerTool == L"ipconfig") {
        // Show Network Information
        ShellExecuteW(NULL, L"open", L"cmd.exe", L"/k ipconfig /all", NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Show Network Information", L"Network configuration displayed", CommandCategory::NETWORK_TOOLS);
        return true;
    }
    
    return false;
}

bool CommandManager::ExecuteDeveloperCommand(const std::wstring& devTool) {
    std::wstring lowerTool = devTool;
    std::transform(lowerTool.begin(), lowerTool.end(), lowerTool.begin(), ::towlower);
    
    if (lowerTool.empty() || lowerTool == L"powershell" || lowerTool == L"ps") {
        // Open PowerShell
        ShellExecuteW(NULL, L"open", L"powershell.exe", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Open PowerShell", L"PowerShell terminal opened", CommandCategory::DEVELOPER_TOOLS);
        return true;
    }
    else if (lowerTool == L"gitbash" || lowerTool == L"git" || lowerTool == L"bash") {
        // Open Git Bash
        std::vector<std::wstring> gitBashPaths = {
            L"C:\\Program Files\\Git\\bin\\bash.exe",
            L"C:\\Program Files (x86)\\Git\\bin\\bash.exe"
        };
        
        bool found = false;
        for (const auto& path : gitBashPaths) {
            if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                found = true;
                break;
            }
        }
        
        if (found) {
            m_executionHistory.AddExecution(L"Open Git Bash", L"Git Bash terminal opened", CommandCategory::DEVELOPER_TOOLS);
            return true;
        } else {
            MessageBoxW(NULL, L"Git Bash nicht gefunden. Bitte installieren Sie Git für Windows.", L"WinPal - Git Bash", MB_ICONWARNING | MB_OK);
            return false;
        }
    }
    
    return false;
}

bool CommandManager::ExecuteClipboardCommand(const std::wstring& action) {
    std::wstring lowerAction = action;
    std::transform(lowerAction.begin(), lowerAction.end(), lowerAction.begin(), ::towlower);
    
    if (lowerAction.empty() || lowerAction == L"clear" || lowerAction == L"empty") {
        // Clear Clipboard
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            CloseClipboard();
            m_executionHistory.AddExecution(L"Clear Clipboard", L"Clipboard contents cleared", CommandCategory::CLIPBOARD_MANAGER);
            MessageBoxW(NULL, L"Zwischenablage erfolgreich geleert!", L"WinPal - Zwischenablage", MB_ICONINFORMATION | MB_OK);
            return true;
        } else {
            MessageBoxW(NULL, L"Fehler beim Zugriff auf die Zwischenablage!", L"WinPal - Fehler", MB_ICONERROR | MB_OK);
            return false;
        }
    }
    
    return false;
}

bool CommandManager::ExecuteSettingsCommand(const std::wstring& settingName) {
    if (settingName.empty()) {
        // Open main Windows Settings
        ShellExecuteW(NULL, L"open", L"ms-settings:", NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Windows Settings", L"Main Windows Settings opened", CommandCategory::SETTINGS);
        return true;
    }
    
    std::wstring lowerSetting = settingName;
    std::transform(lowerSetting.begin(), lowerSetting.end(), lowerSetting.begin(), ::towlower);
    
    // Map common setting names to their ms-settings URIs
    std::map<std::wstring, std::wstring> settingsMap = {
        {L"display", L"ms-settings:display"},
        {L"sound", L"ms-settings:sound"},
        {L"bluetooth", L"ms-settings:bluetooth"},
        {L"wifi", L"ms-settings:network-wifi"},
        {L"network", L"ms-settings:network-status"},
        {L"apps", L"ms-settings:appsfeatures"},
        {L"system", L"ms-settings:about"},
        {L"personalization", L"ms-settings:personalization"},
        {L"background", L"ms-settings:personalization-background"},
        {L"privacy", L"ms-settings:privacy"},
        {L"updates", L"ms-settings:windowsupdate"},
        {L"power", L"ms-settings:powersleep"},
        {L"storage", L"ms-settings:storagesense"},
        {L"accounts", L"ms-settings:yourinfo"},
        {L"time", L"ms-settings:dateandtime"},
        {L"language", L"ms-settings:regionlanguage"},
        {L"ease", L"ms-settings:easeofaccess"},
        {L"taskbar", L"ms-settings:taskbar"},
        {L"startup", L"ms-settings:startupapps"}
    };
    
    auto it = settingsMap.find(lowerSetting);
    if (it != settingsMap.end()) {
        ShellExecuteW(NULL, L"open", it->second.c_str(), NULL, NULL, SW_SHOWNORMAL);
        m_executionHistory.AddExecution(L"Settings: " + settingName, L"Windows setting opened: " + it->second, CommandCategory::SETTINGS);
        return true;
    }
    
    // Fallback: try to search in registered settings commands
    for (const auto& command : m_commands) {
        if (command->GetCategory() == CommandCategory::SETTINGS) {
            std::wstring commandName = command->GetName();
            std::wstring lowerCommandName = commandName;
            std::transform(lowerCommandName.begin(), lowerCommandName.end(), lowerCommandName.begin(), ::towlower);
            
            if (lowerCommandName.find(lowerSetting) != std::wstring::npos) {
                command->Execute();
                m_executionHistory.AddExecution(commandName, command->GetDescription(), CommandCategory::SETTINGS);
                return true;
            }
        }
    }
    
    return false;
}
