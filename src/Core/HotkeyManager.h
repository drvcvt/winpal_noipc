#pragma once

#include <windows.h>

class HotkeyManager {
public:
    static const int PALETTE_HOTKEY_ID = 1;

    // Registers the global hotkey Alt+Ctrl+P
    bool RegisterHotkeys(HWND hwnd);

    // Unregisters the hotkey
    void UnregisterHotkeys(HWND hwnd);
};
