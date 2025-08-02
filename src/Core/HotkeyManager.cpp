#include "HotkeyManager.h"
#include <string>
#include <fstream>
#include <sstream>
#include <regex>

HotkeyManager::HotkeyManager() {
    LoadHotkeyConfiguration();
}

void HotkeyManager::LoadHotkeyConfiguration() {
    m_hotkeys.clear();

    // Attempt to load hotkey definitions from external JSON file.
    // The file is expected to contain an array of objects with the
    // fields: id, modifiers, vkCode and commandName.
    std::ifstream file("hotkeys.json");
    if (file) {
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        std::regex objectRegex("\\{[^}]*\\}");
        auto begin = std::sregex_iterator(content.begin(), content.end(), objectRegex);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            std::string obj = (*it).str();
            std::smatch match;

            Hotkey hk{};

            if (!std::regex_search(obj, match, std::regex("\"id\"\\s*:\\s*(\\d+)")))
                continue;
            hk.id = std::stoi(match[1]);

            if (!std::regex_search(obj, match, std::regex("\"modifiers\"\\s*:\\s*(\\d+)")))
                continue;
            hk.modifiers = static_cast<UINT>(std::stoi(match[1]));

            if (!std::regex_search(obj, match, std::regex("\"vkCode\"\\s*:\\s*(\\d+)")))
                continue;
            hk.vkCode = static_cast<UINT>(std::stoi(match[1]));

            if (!std::regex_search(obj, match, std::regex("\"commandName\"\\s*:\\s*\"([^\"]*)\"")))
                continue;
            std::string cmd = match[1];
            hk.commandName = std::wstring(cmd.begin(), cmd.end());

            m_hotkeys.push_back(hk);
        }
    }

    // Fallback to default hotkeys if configuration is missing or invalid.
    if (m_hotkeys.empty()) {
        // Global palette hotkey: Alt + Space
        m_hotkeys.push_back({PALETTE_HOTKEY_ID, MOD_ALT, VK_SPACE, L""});

        // Additional user hotkeys
        m_hotkeys.push_back({2, MOD_ALT, 'C', L"calculator"});
        m_hotkeys.push_back({3, MOD_ALT, 'N', L"notepad"});
        m_hotkeys.push_back({4, MOD_ALT, 'T', L"task manager"});
    }
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
