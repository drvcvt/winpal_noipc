#include "ExecutionHistory.h"

void ExecutionHistory::AddExecution(const ICommand* command) {
    if (command == nullptr) return;

    // Neuen Eintrag am Anfang hinzufügen
    m_history.insert(m_history.begin(), HistoryEntry(
        command->GetName(),
        command->GetDescription(),
        command->GetCategory()
    ));

    // Auf maximale Größe begrenzen
    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.resize(MAX_HISTORY_SIZE);
    }
}

void ExecutionHistory::AddExecution(const std::wstring& name, const std::wstring& description, CommandCategory category) {
    // Neuen Eintrag am Anfang hinzufügen
    m_history.insert(m_history.begin(), HistoryEntry(name, description, category));

    // Auf maximale Größe begrenzen
    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.resize(MAX_HISTORY_SIZE);
    }
}

void ExecutionHistory::AddPowerShellExecution(const std::wstring& command) {
    // Neuen PowerShell-Eintrag am Anfang hinzufügen
    m_history.insert(m_history.begin(), HistoryEntry(
        L"PowerShell: " + command,
        L"Direkte PowerShell-Ausführung",
        CommandCategory::DEVELOPER_TOOLS
    ));

    // Auf maximale Größe begrenzen
    if (m_history.size() > MAX_HISTORY_SIZE) {
        m_history.resize(MAX_HISTORY_SIZE);
    }
}

const std::vector<HistoryEntry>& ExecutionHistory::GetHistory() const {
    return m_history;
}

bool ExecutionHistory::IsEmpty() const {
    return m_history.empty();
}

void ExecutionHistory::Clear() {
    m_history.clear();
} 