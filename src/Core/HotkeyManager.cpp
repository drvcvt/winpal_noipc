#include "HotkeyManager.h"
#include <string>

HotkeyManager::HotkeyManager() {
    LoadHotkeyConfiguration();
}

void HotkeyManager::LoadHotkeyConfiguration() {
    m_hotkeys.clear();
    
    // Neuer globaler Hotkey zum Öffnen der Palette: Alt + Space
    m_hotkeys.push_back({PALETTE_HOTKEY_ID, MOD_ALT, VK_SPACE, L""});

    // Benutzerdefinierte Hotkeys
    m_hotkeys.push_back({2, MOD_ALT, 'C', L"calculator"});
    m_hotkeys.push_back({3, MOD_ALT, 'N', L"notepad"});
    m_hotkeys.push_back({4, MOD_ALT, 'T', L"task manager"});
}

bool HotkeyManager::RegisterHotkeys(HWND hwnd) {
    for (const auto& hotkey : m_hotkeys) {
        if (!RegisterHotKey(hwnd, hotkey.id, hotkey.modifiers, hotkey.vkCode)) {
            DWORD errorCode = GetLastError();
            std::wstring errorMsg = L"Failed to register hotkey ID: " + std::to_wstring(hotkey.id) + L".\n";
            errorMsg += L"Modifiers: " + std::to_wstring(hotkey.modifiers) + L", Key: " + std::to_wstring(hotkey.vkCode) + L".\n";
            
            if (errorCode == ERROR_HOTKEY_ALREADY_REGISTERED) {
                errorMsg += L"Error: Hotkey is already registered by another application.";
            } else {
                errorMsg += L"Windows Error Code: " + std::to_wstring(errorCode);
            }
            
            MessageBoxW(NULL, errorMsg.c_str(), L"Hotkey Registration Error", MB_ICONERROR | MB_OK);
            return false; // Stop on first error
        }
    }
    return true;
}

void HotkeyManager::UnregisterHotkeys(HWND hwnd) {
    for (const auto& hotkey : m_hotkeys) {
        UnregisterHotKey(hwnd, hotkey.id);
    }
}

std::wstring HotkeyManager::FindCommandForHotkey(int hotkeyId) const {
    for (const auto& hotkey : m_hotkeys) {
        if (hotkey.id == hotkeyId) {
            return hotkey.commandName;
        }
    }
    return L"";
}
