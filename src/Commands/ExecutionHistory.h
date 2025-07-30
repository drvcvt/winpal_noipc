#pragma once

#include "ICommand.h"
#include <vector>
#include <memory>
#include <string>
#include <chrono>

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
    static const size_t MAX_HISTORY_SIZE = 4;

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
}; 