#pragma once

#include <windows.h>
#include <vector>
#include <string>

// Struktur zur Definition eines einzelnen Hotkeys und seiner Aktion
struct Hotkey {
    int id;
    UINT modifiers;
    UINT vkCode;
    std::wstring commandName; // Name des Befehls, der ausgeführt werden soll
};

class HotkeyManager {
public:
    // Statische ID für den globalen Palette-Hotkey beibehalten
    static const int PALETTE_HOTKEY_ID = 1;

    HotkeyManager();

    // Registriert alle geladenen Hotkeys
    bool RegisterHotkeys(HWND hwnd);

    // Deregistriert alle Hotkeys
    void UnregisterHotkeys(HWND hwnd);

    // Findet den Befehlsnamen für eine ausgelöste Hotkey-ID
    std::wstring FindCommandForHotkey(int hotkeyId) const;

private:
    std::vector<Hotkey> m_hotkeys;

    // Liest Hotkeys aus einer Konfigurationsdatei und befüllt m_hotkeys
    void LoadHotkeyConfiguration();
};
