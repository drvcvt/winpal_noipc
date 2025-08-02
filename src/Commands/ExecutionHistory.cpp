#include "ExecutionHistory.h"

#include <fstream>
#include <filesystem>
#include <cstdlib>

using namespace std::chrono;

namespace {
// Helper to build the base application data directory
std::filesystem::path GetBaseDir() {
    std::filesystem::path base;
    if (const char* appData = std::getenv("APPDATA")) {
        base = appData;
    } else {
        base = std::filesystem::current_path();
    }
    base /= "WinPal";
    std::filesystem::create_directories(base);
    return base;
}
}

ExecutionHistory::ExecutionHistory() : m_maxHistorySize(4) {
    LoadSettings();
    LoadHistory();
}

void ExecutionHistory::AddExecution(const ICommand* command) {
    if (command == nullptr) return;

    // Neuen Eintrag am Anfang hinzufügen
    m_history.insert(m_history.begin(),
        HistoryEntry(command->GetName(), command->GetDescription(), command->GetCategory()));

    // Auf maximale Größe begrenzen
    if (m_history.size() > m_maxHistorySize) {
        m_history.resize(m_maxHistorySize);
    }

    SaveHistory();
}

void ExecutionHistory::AddExecution(const std::wstring& name, const std::wstring& description, CommandCategory category) {
    // Neuen Eintrag am Anfang hinzufügen
    m_history.insert(m_history.begin(), HistoryEntry(name, description, category));

    // Auf maximale Größe begrenzen
    if (m_history.size() > m_maxHistorySize) {
        m_history.resize(m_maxHistorySize);
    }

    SaveHistory();
}

void ExecutionHistory::AddPowerShellExecution(const std::wstring& command) {
    // Neuen PowerShell-Eintrag am Anfang hinzufügen
    m_history.insert(m_history.begin(),
        HistoryEntry(L"PowerShell: " + command, L"Direkte PowerShell-Ausführung", CommandCategory::DEVELOPER_TOOLS));

    // Auf maximale Größe begrenzen
    if (m_history.size() > m_maxHistorySize) {
        m_history.resize(m_maxHistorySize);
    }

    SaveHistory();
}

const std::vector<HistoryEntry>& ExecutionHistory::GetHistory() const {
    return m_history;
}

bool ExecutionHistory::IsEmpty() const {
    return m_history.empty();
}

void ExecutionHistory::Clear() {
    m_history.clear();
    SaveHistory();
}

// --- Persistence helpers ---

void ExecutionHistory::SaveHistory() const {
    std::wofstream file(GetHistoryFilePath());
    if (!file.is_open()) return;

    for (const auto& entry : m_history) {
        file << entry.commandName << L'\n'
             << entry.commandDescription << L'\n'
             << static_cast<int>(entry.category) << L'\n'
             << duration_cast<milliseconds>(entry.executionTime.time_since_epoch()).count() << L'\n';
    }
}

void ExecutionHistory::LoadHistory() {
    std::wifstream file(GetHistoryFilePath());
    if (!file.is_open()) return;

    m_history.clear();
    std::wstring name, desc, catStr, timeStr;
    while (std::getline(file, name) && std::getline(file, desc) &&
           std::getline(file, catStr) && std::getline(file, timeStr)) {
        try {
            CommandCategory cat = static_cast<CommandCategory>(std::stoi(catStr));
            long long ms = std::stoll(timeStr);
            HistoryEntry entry(name, desc, cat);
            entry.executionTime = system_clock::time_point(milliseconds(ms));
            m_history.push_back(entry);
            if (m_history.size() >= m_maxHistorySize) break;
        } catch (...) {
            // ignore malformed entries
        }
    }
}

void ExecutionHistory::LoadSettings() {
    std::wifstream file(GetSettingsFilePath());
    if (!file.is_open()) return;

    std::wstring line;
    while (std::getline(file, line)) {
        size_t pos = line.find(L'=');
        if (pos == std::wstring::npos) continue;
        std::wstring key = line.substr(0, pos);
        std::wstring value = line.substr(pos + 1);
        if (key == L"max_history_size") {
            try {
                size_t val = std::stoul(value);
                if (val > 0) m_maxHistorySize = val;
            } catch (...) {
                // ignore invalid values
            }
        }
    }
}

std::filesystem::path ExecutionHistory::GetHistoryFilePath() const {
    return GetBaseDir() / "history.txt";
}

std::filesystem::path ExecutionHistory::GetSettingsFilePath() const {
    return GetBaseDir() / "settings.txt";
}

