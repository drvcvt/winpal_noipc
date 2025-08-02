#pragma once

#include "ICommand.h"
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>

struct HistoryEntry {
    std::wstring commandName;
    std::wstring commandDescription;
    CommandCategory category;
    std::chrono::system_clock::time_point executionTime;

    // Default-Konstruktor
    HistoryEntry() : commandName(L""), commandDescription(L""), category(CommandCategory::UNKNOWN), 
                     executionTime(std::chrono::system_clock::now()) {}

    // Parametrisierter Konstruktor
    HistoryEntry(const std::wstring& name, const std::wstring& desc, CommandCategory cat)
        : commandName(name), commandDescription(desc), category(cat), executionTime(std::chrono::system_clock::now()) {}
};

class ExecutionHistory {
public:
    ExecutionHistory();

    // Fügt einen Command zum Verlauf hinzu
    void AddExecution(const ICommand* command);

    // Fügt eine direkte Ausführung zum Verlauf hinzu (für Shebang-Commands)
    void AddExecution(const std::wstring& name, const std::wstring& description, CommandCategory category);

    // Fügt eine PowerShell-Ausführung zum Verlauf hinzu
    void AddPowerShellExecution(const std::wstring& command);

    // Gibt die History-Einträge zurück (neueste zuerst)
    const std::vector<HistoryEntry>& GetHistory() const;

    // Prüft ob History leer ist
    bool IsEmpty() const;

    // Löscht die gesamte History
    void Clear();

private:
    std::vector<HistoryEntry> m_history;
    size_t m_maxHistorySize;

    void SaveHistory() const;
    void LoadHistory();
    void LoadSettings();
    std::filesystem::path GetHistoryFilePath() const;
    std::filesystem::path GetSettingsFilePath() const;
};
