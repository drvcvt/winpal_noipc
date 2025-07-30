#pragma once

#include <string>

// Kategorien für bessere Organisation
enum class CommandCategory {
    SETTINGS,
    PROCESS_TOOLS,
    FILE_TOOLS,
    APPLICATION_LAUNCHER,
    CLIPBOARD_MANAGER,
    NETWORK_TOOLS,
    DEVELOPER_TOOLS,
    SYSTEM_INFO,
    SYSTEM_CONTROL,
    UNKNOWN
};

// Eine "reine virtuelle" Klasse, die als Interface dient.
class ICommand {
public:
    virtual ~ICommand() = default; // Wichtig bei Vererbung!

    // Der Name, der in der Liste angezeigt wird (z.B. "Prozess beenden")
    virtual std::wstring GetName() const = 0;

    // Eine kurze Beschreibung, die vielleicht als Tooltip erscheint
    virtual std::wstring GetDescription() const = 0;

    // Die Kategorie für bessere Organisation
    virtual CommandCategory GetCategory() const = 0;

    // Die eigentliche Logik, die ausgeführt wird
    virtual void Execute() = 0;
};
