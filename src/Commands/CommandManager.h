#pragma once

#include "ICommand.h"
#include "ExecutionHistory.h"
#include <vector>
#include <memory>
#include <string>
#include <map>

// Neue Struktur für erweiterte Suchergebnisse
struct SearchResult {
    ICommand* command;
    double relevanceScore;
    std::wstring matchedText;
    enum MatchType {
        EXACT_NAME,
        STARTS_WITH_NAME,
        CONTAINS_NAME,
        EXACT_DESCRIPTION,
        CONTAINS_DESCRIPTION,
        FUZZY_NAME,
        FUZZY_DESCRIPTION,
        CATEGORY_MATCH
    } matchType;
    
    SearchResult(ICommand* cmd, double score, const std::wstring& matched, MatchType type)
        : command(cmd), relevanceScore(score), matchedText(matched), matchType(type) {}
};

class CommandManager
{
public:
    void RegisterCommand(std::unique_ptr<ICommand> command);
    void RegisterSettingsCommands();
    void RegisterFileToolsCommands();
    void RegisterApplicationLauncherCommands();
    void RegisterSystemInfoCommands();
    void RegisterNetworkToolsCommands();
    void RegisterClipboardManagerCommands();
    void RegisterDeveloperToolsCommands();
    void RegisterAllPlugins();
    
    // Verbesserte Suchfunktionen
    std::vector<ICommand*> FindCommands(const std::wstring& query);
    std::vector<SearchResult> FindCommandsWithRelevance(const std::wstring& query);
    std::vector<std::wstring> GetSearchSuggestions(const std::wstring& partialQuery, int maxSuggestions = 8);
    
    std::vector<ICommand*> GetCommandsByCategory(CommandCategory category);

    // ExecutionHistory Funktionalität
    void ExecuteCommand(ICommand* command);
    void ExecutePowerShellCommand(const std::wstring& command);
    ExecutionHistory& GetExecutionHistory();
    const ExecutionHistory& GetExecutionHistory() const;

    // Neue Shebang-Command-Funktionalität
    bool IsShebangCommand(const std::wstring& input) const;
    bool ExecuteShebangCommand(const std::wstring& input);
    
    // Natürliche Command-Funktionalität (z.B. "launch spotify", "terminate notepad")
    bool IsNaturalCommand(const std::wstring& input) const;
    bool ExecuteNaturalCommand(const std::wstring& input);
    
private:
    std::vector<std::unique_ptr<ICommand>> m_commands;
    ExecutionHistory m_executionHistory;
    
    // Neue Hilfsmethoden für erweiterte Suche
    double CalculateRelevanceScore(const std::wstring& commandName, const std::wstring& commandDesc, 
                                   const std::wstring& query, SearchResult::MatchType& matchType, 
                                   std::wstring& matchedText);
    double CalculateFuzzyScore(const std::wstring& text, const std::wstring& query);
    double CalculateFrequencyBoost(ICommand* command);
    std::vector<std::wstring> SplitQuery(const std::wstring& query);
    bool ContainsIgnoreCase(const std::wstring& text, const std::wstring& search);
    std::wstring ToLower(const std::wstring& text);
    
    // Hilfsmethoden für Shebang-Commands
    bool ExecuteLaunchCommand(const std::wstring& appName);
    bool ExecuteTerminateCommand(const std::wstring& processName);
    bool ExecuteFileToolCommand(const std::wstring& target);
    bool ExecuteSystemInfoCommand(const std::wstring& infoType);
    bool ExecuteNetworkCommand(const std::wstring& networkTool);
    bool ExecuteDeveloperCommand(const std::wstring& devTool);
    bool ExecuteClipboardCommand(const std::wstring& action);
    bool ExecuteSettingsCommand(const std::wstring& settingName);
    
    // Hilfsmethoden für natürliche Commands
    std::pair<std::wstring, std::wstring> ParseNaturalCommand(const std::wstring& input) const;
};
