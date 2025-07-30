#include "HotkeyManager.h"

bool HotkeyManager::RegisterHotkeys(HWND hwnd) {
    // Register Alt+Ctrl+P as the hotkey
    if (RegisterHotKey(hwnd, PALETTE_HOTKEY_ID, MOD_ALT | MOD_CONTROL, 0x50)) { // 0x50 is the virtual key code for 'P'
        return true;
    }
    return false;
}

void HotkeyManager::UnregisterHotkeys(HWND hwnd) {
    UnregisterHotKey(hwnd, PALETTE_HOTKEY_ID);
}
