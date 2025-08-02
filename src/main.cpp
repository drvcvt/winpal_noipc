#define _WIN32_WINNT 0x0A00 // Define minimum Windows version for modern APIs
#include <windows.h>
#include <tlhelp32.h> // For process enumeration
#ifdef _WIN32
#include <dwmapi.h> // Include DWM API header
#endif
#include <vector>
#include <string>
#include <memory>
#include <gdiplus.h>
#include <chrono>
#include <map>
#include <algorithm>
#include <cwctype>
#include "Core/HotkeyManager.h"
#include "Core/GuiManager.h"
#include "Commands/CommandManager.h"
#include "Commands/ICommand.h"
#include "Commands/ExecutionHistory.h"
#include "Plugins/ProcessTools/EnterProcessModeCommand.h"
#include "Plugins/ProcessTools/TerminateProcessCommand.h"
#include "Plugins/ProcessTools/OpenProcessPathCommand.h"
#include "Plugins/ApplicationLauncher/GenericLaunchCommand.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib") // Link against the DWM API

using namespace Gdiplus;

// Global variables
HWND g_hwnd;
HotkeyManager g_hotkeyManager;
GuiManager g_guiManager;
CommandManager g_commandManager;
std::wstring g_inputBuffer;
std::vector<ICommand*> g_foundCommands;
int g_selectedCommand = 0;
bool g_isWindowVisible = false;
HFONT g_hFont = NULL;
HFONT g_hDescFont = NULL;
HFONT g_hIconFont = NULL; // Font for icons
ULONG_PTR g_gdiplusToken;

// Einfache, begrenzte Suche für maximale Performance
const int MAX_SEARCH_RESULTS = 15;  // Allow up to 15 results for better coverage

// Autocomplete-Feature Variablen
std::vector<std::wstring> g_autocompleteSuggestions;
int g_selectedSuggestion = -1; // -1 bedeutet keine Auswahl
bool g_isAutocompleteMode = false;
std::wstring g_originalInput; // Speichert die ursprüngliche Eingabe

// Cursor state (animation variables removed)
bool g_cursorVisible = true;
std::chrono::steady_clock::time_point g_lastCursorBlink;

// Modern color palette (PowerToys inspired)
const COLORREF COLOR_BG_MAIN = RGB(45, 45, 45); // Main background (dark grey)
const COLORREF COLOR_BG_INPUT = RGB(30, 30, 30); // Input field background
const COLORREF COLOR_BG_SELECTED = RGB(0, 120, 215); // Selected item background (blue)
const COLORREF COLOR_BORDER = RGB(80, 80, 80); // Border color
const COLORREF COLOR_TEXT_PRIMARY = RGB(242, 242, 242); // Primary text (off-white)
const COLORREF COLOR_TEXT_SECONDARY = RGB(170, 170, 170); // Secondary text (placeholder)
const COLORREF COLOR_TEXT_INPUT = RGB(242, 242, 242); // Input text
const COLORREF COLOR_CURSOR = RGB(242, 242, 242); // Text cursor
// const COLORREF COLOR_HIGHLIGHT = RGB(90, 90, 90); // Highlight color for selection border (grey)

// Category info structure and mapping
struct CategoryInfo {
    std::wstring icon;
    std::wstring name;
    // COLORREF color; // Color is no longer needed per category
};

std::map<CommandCategory, CategoryInfo> g_categories = {
    {CommandCategory::SETTINGS, {L"C", L"Settings"}},
    {CommandCategory::PROCESS_TOOLS, {L"P", L"Process Tools"}},
    {CommandCategory::FILE_TOOLS, {L"F", L"File Tools"}},
    {CommandCategory::APPLICATION_LAUNCHER, {L"A", L"Apps"}},
    {CommandCategory::CLIPBOARD_MANAGER, {L"B", L"Clipboard"}},
    {CommandCategory::NETWORK_TOOLS, {L"N", L"Network"}},
    {CommandCategory::DEVELOPER_TOOLS, {L"D", L"Dev Tools"}},
    {CommandCategory::SYSTEM_INFO, {L"S", L"System Info"}},
    {CommandCategory::SYSTEM_CONTROL, {L"Y", L"System"}},
    {CommandCategory::UNKNOWN, {L"U", L"Unknown"}}
};

// History Icon (Einfacher Buchstabe H für History)
const std::wstring HISTORY_ICON = L"H";

// Helper function to get icon from application commands
HICON GetIconFromCommand(ICommand* cmd) {
    // Check if this is a GenericLaunchCommand and try to get the icon
    if (cmd->GetCategory() == CommandCategory::APPLICATION_LAUNCHER) {
        GenericLaunchCommand* launchCmd = dynamic_cast<GenericLaunchCommand*>(cmd);
        if (launchCmd) {
            auto apps = launchCmd->GetMatchingApplications();
            if (!apps.empty() && apps[0].hIcon) {
                return apps[0].hIcon;
            }
        }
    }
    return nullptr;
}

// Helper function to create rounded rectangle with opacity
void DrawRoundedRect(HDC hdc, RECT rect, int radius, COLORREF fillColor, COLORREF borderColor = 0, int borderWidth = 0, float opacity = 1.0f) {
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // Apply opacity
    Color fill(static_cast<int>(255 * opacity), GetRValue(fillColor), GetGValue(fillColor), GetBValue(fillColor));
    SolidBrush brush(fill);

    GraphicsPath path;
    path.AddArc(rect.left, rect.top, radius * 2, radius * 2, 180, 90);
    path.AddArc(rect.right - radius * 2, rect.top, radius * 2, radius * 2, 270, 90);
    path.AddArc(rect.right - radius * 2, rect.bottom - radius * 2, radius * 2, radius * 2, 0, 90);
    path.AddArc(rect.left, rect.bottom - radius * 2, radius * 2, radius * 2, 90, 90);
    path.CloseFigure();

    graphics.FillPath(&brush, &path);

    if (borderWidth > 0 && borderColor != 0) {
        Color borderCol(static_cast<int>(255 * opacity), GetRValue(borderColor), GetGValue(borderColor), GetBValue(borderColor));
        Pen pen(borderCol, static_cast<REAL>(borderWidth));
        graphics.DrawPath(&pen, &path);
    }
}

// Funktion zur dynamischen Anpassung der Fenstergröße
void UpdateWindowSize() {
    if (!g_isWindowVisible) return; // Nur anpassen wenn Fenster sichtbar ist
    
    int windowWidth = 750;  // Standard width for a command palette
    int windowHeight = 65; // Height for input bar + padding
    
    // Berechne Höhe basierend auf Suchergebnissen, Autocomplete oder History
    if (g_isAutocompleteMode && !g_autocompleteSuggestions.empty()) {
        // Height for input bar + autocomplete suggestions (max 8, smaller items) + hint
        int suggestionCount = min((int)g_autocompleteSuggestions.size(), 8);
        windowHeight = 65 + (35 + 1) * suggestionCount + 25; // +25 for hint text
    } else if (!g_foundCommands.empty()) {
        // Height for input bar + search results (up to MAX_SEARCH_RESULTS)
        int displayedResults = min((int)g_foundCommands.size(), MAX_SEARCH_RESULTS);
        windowHeight = 65 + (55 + 2) * displayedResults;
    } else if (g_inputBuffer.empty()) {
        // Zeige History wenn keine Eingabe vorhanden
        const ExecutionHistory& history = g_commandManager.GetExecutionHistory();
        if (!history.IsEmpty()) {
            // Height for input bar + history entries (up to MAX_SEARCH_RESULTS)
            int historyCount = min((int)history.GetHistory().size(), MAX_SEARCH_RESULTS);
            windowHeight = 65 + (55 + 2) * historyCount;
        }
    }
    
    // Position beibehalten, nur Größe ändern
    RECT rect;
    GetWindowRect(g_hwnd, &rect);
    
    SetWindowPos(g_hwnd, HWND_TOPMOST, rect.left, rect.top, windowWidth, windowHeight, SWP_NOACTIVATE);
}

// Generiere Autocomplete-Vorschläge für Shebang-Commands
std::vector<std::wstring> GetShebangSuggestions(const std::wstring& input) {
    std::vector<std::wstring> suggestions;
    
    if (input.empty() || input[0] != L'!') {
        return suggestions;
    }
    
    // Wenn nur "!" eingegeben wurde, zeige alle verfügbaren Shebangs
    if (input.length() == 1) {
        suggestions.push_back(L"!l "); // Launch
        suggestions.push_back(L"!t "); // Terminate
        suggestions.push_back(L"!f "); // File Tools
        suggestions.push_back(L"!s "); // System Info
        suggestions.push_back(L"!n "); // Network Tools
        suggestions.push_back(L"!d "); // Developer Tools
        suggestions.push_back(L"!c "); // Clipboard Manager
        suggestions.push_back(L"!z "); // Settings
        return suggestions;
    }
    
    // Wenn ein Shebang-Typ eingegeben wurde, suche nach passenden Anwendungen/Commands
    if (input.length() >= 2) {
        wchar_t commandType = ::towlower(input[1]);
        std::wstring searchTerm;
        
        // Extrahiere Suchterm nach dem Shebang (z.B. "!l s" -> "s")
        if (input.length() > 3 && input[2] == L' ') {
            searchTerm = input.substr(3);
        }
        
        switch (commandType) {
            case L'l': // Launch command - SCHNELLE statische Vorschläge
            {
                // KEINE schweren Operationen mehr - nur vordefinierte beliebte Apps
                static const std::vector<std::wstring> popularApps = {
                    L"notepad", L"calculator", L"task manager", L"spotify", 
                    L"chrome", L"firefox", L"code", L"steam", L"explorer",
                    L"paint", L"word", L"excel", L"outlook", L"teams"
                };
                
                if (!searchTerm.empty()) {
                    // Filtere nur aus der statischen Liste - keine System-Suche
                    std::wstring lowerSearchTerm = searchTerm;
                    std::transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), ::towlower);
                    
                    for (const auto& app : popularApps) {
                        std::wstring lowerApp = app;
                        std::transform(lowerApp.begin(), lowerApp.end(), lowerApp.begin(), ::towlower);
                        
                        if (lowerApp.find(lowerSearchTerm) != std::wstring::npos) {
                            suggestions.push_back(L"!l " + app);
                            if (suggestions.size() >= 6) break; // Begrenze Ergebnisse
                        }
                    }
                } else {
                    // Zeige nur die ersten 6 beliebten Apps
                    for (size_t i = 0; i < popularApps.size() && i < 6; ++i) {
                        suggestions.push_back(L"!l " + popularApps[i]);
                    }
                }
                break;
            }
            case L't': // Terminate command - SCHNELLE statische Vorschläge
            {
                // KEINE Prozess-Auflistung mehr - nur vordefinierte häufige Prozesse
                static const std::vector<std::wstring> commonProcesses = {
                    L"notepad", L"chrome", L"firefox", L"spotify", 
                    L"code", L"steam", L"explorer", L"word", L"excel"
                };
                
                if (!searchTerm.empty()) {
                    // Filtere nur aus der statischen Liste
                    std::wstring lowerSearchTerm = searchTerm;
                    std::transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), ::towlower);
                    
                    for (const auto& process : commonProcesses) {
                        std::wstring lowerProcess = process;
                        std::transform(lowerProcess.begin(), lowerProcess.end(), lowerProcess.begin(), ::towlower);
                        
                        if (lowerProcess.find(lowerSearchTerm) != std::wstring::npos) {
                            suggestions.push_back(L"!t " + process);
                            if (suggestions.size() >= 6) break; // Begrenze Ergebnisse
                        }
                    }
                } else {
                    // Zeige nur die ersten 4 häufigen Prozesse
                    for (size_t i = 0; i < commonProcesses.size() && i < 4; ++i) {
                        suggestions.push_back(L"!t " + commonProcesses[i]);
                    }
                }
                break;
            }
            case L'f': // File Tools
                suggestions.push_back(L"!f explorer");
                suggestions.push_back(L"!f downloads");
                suggestions.push_back(L"!f desktop");
                break;
            case L's': // System Info
                suggestions.push_back(L"!s info");
                suggestions.push_back(L"!s disk");
                break;
            case L'n': // Network Tools
                suggestions.push_back(L"!n info");
                suggestions.push_back(L"!n ping google.com");
                suggestions.push_back(L"!n ping 8.8.8.8");
                break;
            case L'd': // Developer Tools
                suggestions.push_back(L"!d git");
                suggestions.push_back(L"!d powershell");
                break;
            case L'c': // Clipboard
                suggestions.push_back(L"!c clear");
                break;
            case L'z': // Settings
                suggestions.push_back(L"!z settings");
                break;
        }
    }
    
    return suggestions;
}

// Extrem schnelle, begrenzte Suche für Live-Updates
void UpdateFoundCommands(const std::wstring& searchTerm) {
    // Reset Autocomplete-Zustand
    g_isAutocompleteMode = false;
    g_selectedSuggestion = -1;
    g_autocompleteSuggestions.clear();
    
    // Wenn das Suchfeld leer ist, zeige keine Commands an (damit der Verlauf angezeigt wird)
    if (searchTerm.empty()) {
        g_foundCommands.clear();
        g_selectedCommand = 0;
        UpdateWindowSize(); // Fenstergröße anpassen
        return;
    }
    
    // Prüfe auf Shebang-Commands und generiere Autocomplete-Vorschläge
    if (!searchTerm.empty() && searchTerm[0] == L'!') {
        g_autocompleteSuggestions = GetShebangSuggestions(searchTerm);
        if (!g_autocompleteSuggestions.empty()) {
            g_isAutocompleteMode = true;
            g_originalInput = searchTerm;
        }
        UpdateWindowSize();
        return;
    }
    
    // Suche über den CommandManager und limitiere die Ergebnisse
    g_foundCommands.clear();
    auto results = g_commandManager.FindCommands(searchTerm);
    for (size_t i = 0; i < results.size() && i < MAX_SEARCH_RESULTS; ++i) {
        g_foundCommands.push_back(results[i]);
    }
    
    g_selectedCommand = 0;
    UpdateWindowSize(); // Fenstergröße anpassen
}


// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
        {
            // Initialize GDI+
            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
            
            // Create Segoe UI fonts
            g_hFont = CreateFontW(
                -18,                    // Height (increased for better visibility)
                0,                      // Width
                0,                      // Escapement
                0,                      // Orientation
                FW_NORMAL,              // Weight
                FALSE,                  // Italic
                FALSE,                  // Underline
                FALSE,                  // StrikeOut
                DEFAULT_CHARSET,        // CharSet
                OUT_DEFAULT_PRECIS,     // OutPrecision
                CLIP_DEFAULT_PRECIS,    // ClipPrecision
                CLEARTYPE_QUALITY,      // Quality
                DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
                L"Segoe UI"             // FaceName
            );
            
            g_hDescFont = CreateFontW(
                -14,                    // Height (increased)
                0,                      // Width
                0,                      // Escapement
                0,                      // Orientation
                FW_NORMAL,              // Weight
                FALSE,                  // Italic
                FALSE,                  // Underline
                FALSE,                  // StrikeOut
                DEFAULT_CHARSET,        // CharSet
                OUT_DEFAULT_PRECIS,     // OutPrecision
                CLIP_DEFAULT_PRECIS,    // ClipPrecision
                CLEARTYPE_QUALITY,      // Quality
                DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
                L"Segoe UI"             // FaceName
            );
            
            g_hIconFont = CreateFontW(
                -20,                    // Height
                0,                      // Width
                0,                      // Escapement
                0,                      // Orientation
                FW_NORMAL,              // Weight
                FALSE,                  // Italic
                FALSE,                  // Underline
                FALSE,                  // StrikeOut
                DEFAULT_CHARSET,        // CharSet
                OUT_DEFAULT_PRECIS,     // OutPrecision
                CLIP_DEFAULT_PRECIS,    // ClipPrecision
                CLEARTYPE_QUALITY,      // Quality
                DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
                L"Segoe UI Symbol"      // FaceName for icons
            );

            g_guiManager.CreateComponents(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
            
            // Start animation timer
            SetTimer(hwnd, 1, 16, NULL); // ~60fps for smooth animations
            g_lastCursorBlink = std::chrono::steady_clock::now();
            break;
        }
        
        case WM_TIMER:
        {
            auto now = std::chrono::steady_clock::now();
            
            // Cursor blinking
            auto cursorElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastCursorBlink).count();
            if (cursorElapsed >= 530) { // Standard cursor blink rate
                g_cursorVisible = !g_cursorVisible;
                g_lastCursorBlink = now;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            
            // Window fade animation removed to prevent flickering
            break;
        }
        
        case WM_ERASEBKGND:
            return 1; // We handle all drawing in WM_PAINT
            
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Double buffering to prevent flickering
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
            HGDIOBJ hOld = SelectObject(hdcMem, hbmMem);

            // --- Main Drawing Logic ---

            // Fill the entire client area with the main background color.
            // DWM will handle the rounded corners, so we don't need to use a transparent key color.
            HBRUSH bgBrush = CreateSolidBrush(COLOR_BG_MAIN);
            FillRect(hdcMem, &rcClient, bgBrush);
            DeleteObject(bgBrush);
            
            // Main window rectangle - No longer needed, DWM handles this.
            // RECT rcMainWindow = { 0, 0, rcClient.right, rcClient.bottom };
            // DrawRoundedRect(hdcMem, rcMainWindow, 8, COLOR_BG_MAIN, 0, 0, 1.0f);
            
            // Input field background
            RECT rcInputBg = { 15, 15, rcClient.right - 15, 50 };
            DrawRoundedRect(hdcMem, rcInputBg, 6, COLOR_BG_INPUT, 0, 0, 1.0f);

            // Input field (integrated into the main window)
            RECT rcInput = { 12, 12, rcClient.right - 12, 52 };
            
            // Select font for input text
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, g_hFont);
            SetBkMode(hdcMem, TRANSPARENT);
            
            // Input text
            RECT rcInputText = { rcInput.left + 15, rcInput.top + 5, rcInput.right - 15, rcInput.bottom - 5 };
            
            std::wstring displayText = g_inputBuffer.empty() ? L"Search for apps, files, settings, and more..." : g_inputBuffer;
            COLORREF inputTextColor = g_inputBuffer.empty() ? COLOR_TEXT_SECONDARY : COLOR_TEXT_INPUT;
            SetTextColor(hdcMem, inputTextColor);
            DrawTextW(hdcMem, displayText.c_str(), -1, &rcInputText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            
            // Draw cursor
            if (g_cursorVisible && g_isWindowVisible) {
                SIZE textSize;
                GetTextExtentPoint32W(hdcMem, g_inputBuffer.c_str(), static_cast<int>(g_inputBuffer.length()), &textSize);
                int cursorX = rcInputText.left + textSize.cx + 2;
                int cursorY1 = rcInputText.top + 8;
                int cursorY2 = rcInputText.bottom - 8;
                
                HPEN cursorPen = CreatePen(PS_SOLID, 2, COLOR_CURSOR);
                HPEN oldPen = (HPEN)SelectObject(hdcMem, cursorPen);
                MoveToEx(hdcMem, cursorX, cursorY1, NULL);
                LineTo(hdcMem, cursorX, cursorY2);
                SelectObject(hdcMem, oldPen);
                DeleteObject(cursorPen);
            }

            // --- Results List Drawing ---
            const ExecutionHistory& history = g_commandManager.GetExecutionHistory();
            bool hasHistory = !history.IsEmpty();
            bool hasSearchResults = !g_foundCommands.empty();
            bool hasSearchText = !g_inputBuffer.empty();

            // Entscheidungslogik: Was soll angezeigt werden?
            bool showSearchResults = hasSearchText && hasSearchResults && !g_isAutocompleteMode;
            bool showHistory = !hasSearchText && hasHistory && !g_isAutocompleteMode;
            bool showNoResults = hasSearchText && !hasSearchResults && !g_isAutocompleteMode;
            bool showAutocomplete = g_isAutocompleteMode && !g_autocompleteSuggestions.empty();

            if (showSearchResults) {
                // Show search results when user has typed something and there are results
                int startY = rcInput.bottom + 10; // Start below the input area
                int itemHeight = 55; 
                int itemSpacing = 2;
                int maxResults = MAX_SEARCH_RESULTS; // Show up to the configured maximum results

                for (int i = 0; i < g_foundCommands.size() && i < maxResults; ++i) {
                    ICommand* cmd = g_foundCommands[i];
                    RECT rcItem = { 12, startY, rcClient.right - 12, startY + itemHeight };

                    // Highlight selected item with a border instead of a full background
                    if (i == g_selectedCommand) {
                        // Highlight selected item
                        DrawRoundedRect(hdcMem, rcItem, 6, COLOR_BG_SELECTED, RGB(90, 90, 90), 2, 1.0f);
                    } else {
                        // Optional: a very subtle background for non-selected items if needed
                        // DrawRoundedRect(hdcMem, rcItem, 6, COLOR_BG_MAIN, 0, 0, 1.0f);
                    }

                    // Draw Icon
                    RECT rcIcon = { rcItem.left + 15, rcItem.top + 15, rcItem.left + 45, rcItem.bottom - 15 };
                    
                    // Try to get a real icon first
                    HICON hIcon = GetIconFromCommand(cmd);
                    if (hIcon) {
                        // Draw the real icon
                        int iconSize = 24; // 24x24 pixel icon
                        int iconX = rcIcon.left + (rcIcon.right - rcIcon.left - iconSize) / 2;
                        int iconY = rcIcon.top + (rcIcon.bottom - rcIcon.top - iconSize) / 2;
                        DrawIconEx(hdcMem, iconX, iconY, hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
                    } else {
                        // Fallback to text icon
                        CategoryInfo catInfo = g_categories[cmd->GetCategory()];
                        SelectObject(hdcMem, g_hIconFont); // Use Icon Font
                        SetTextColor(hdcMem, COLOR_TEXT_SECONDARY); // Icons have a more subtle color now
                        DrawTextW(hdcMem, catInfo.icon.c_str(), -1, &rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }

                    // Draw Command Name
                    RECT rcName = { rcItem.left + 55, rcItem.top + 8, rcItem.right - 15, rcItem.top + 32 };
                    SelectObject(hdcMem, g_hFont); // Switch back to regular font
                    SetTextColor(hdcMem, COLOR_TEXT_PRIMARY);
                    DrawTextW(hdcMem, cmd->GetName().c_str(), -1, &rcName, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    // Draw Command Description
                    RECT rcDesc = { rcItem.left + 55, rcItem.top + 30, rcItem.right - 15, rcItem.bottom - 8 };
                    SelectObject(hdcMem, g_hDescFont); // Switch to description font
                    SetTextColor(hdcMem, COLOR_TEXT_SECONDARY);
                    DrawTextW(hdcMem, cmd->GetDescription().c_str(), -1, &rcDesc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    
                    startY += itemHeight + itemSpacing;
                }
            }
            else if (showHistory) {
                // Show history when input is empty and there is history available
                int startY = rcInput.bottom + 10; // Start below the input area
                int itemHeight = 55; 
                int itemSpacing = 2;
                const auto& historyEntries = history.GetHistory();
                int maxHistory = min((int)historyEntries.size(), MAX_SEARCH_RESULTS);

                for (int i = 0; i < maxHistory; ++i) {
                    const HistoryEntry& entry = historyEntries[i];
                    RECT rcItem = { 12, startY, rcClient.right - 12, startY + itemHeight };

                    // Subtle background for history items
                    RECT rcItemBg = rcItem;
                    rcItemBg.left += 2;
                    rcItemBg.top += 1;
                    rcItemBg.right -= 2;
                    rcItemBg.bottom -= 1;
                    DrawRoundedRect(hdcMem, rcItemBg, 4, RGB(40, 40, 40), 0, 0, 0.7f);

                    // Draw History Icon (rechts)
                    RECT rcHistoryIcon = { rcItem.right - 40, rcItem.top + 15, rcItem.right - 15, rcItem.bottom - 15 };
                    SelectObject(hdcMem, g_hIconFont);
                    SetTextColor(hdcMem, RGB(120, 120, 120)); // Dunkleres Grau für History-Icon
                    DrawTextW(hdcMem, HISTORY_ICON.c_str(), -1, &rcHistoryIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    // Draw Category Icon (links)
                    CategoryInfo catInfo = g_categories[entry.category];
                    RECT rcIcon = { rcItem.left + 15, rcItem.top + 15, rcItem.left + 45, rcItem.bottom - 15 };
                    SelectObject(hdcMem, g_hIconFont);
                    SetTextColor(hdcMem, RGB(140, 140, 140)); // Gedämpfte Farbe für History
                    DrawTextW(hdcMem, catInfo.icon.c_str(), -1, &rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    // Draw Command Name
                    RECT rcName = { rcItem.left + 55, rcItem.top + 8, rcItem.right - 50, rcItem.top + 32 };
                    SelectObject(hdcMem, g_hFont);
                    SetTextColor(hdcMem, RGB(180, 180, 180)); // Gedämpfte Textfarbe für History
                    DrawTextW(hdcMem, entry.commandName.c_str(), -1, &rcName, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    // Draw Command Description
                    RECT rcDesc = { rcItem.left + 55, rcItem.top + 30, rcItem.right - 50, rcItem.bottom - 8 };
                    SelectObject(hdcMem, g_hDescFont);
                    SetTextColor(hdcMem, RGB(120, 120, 120)); // Noch gedämpftere Farbe für Description
                    DrawTextW(hdcMem, entry.commandDescription.c_str(), -1, &rcDesc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    
                    startY += itemHeight + itemSpacing;
                }
            }
            else if (showAutocomplete) {
                // Show autocomplete suggestions
                int startY = rcInput.bottom + 10; // Start below the input area
                int itemHeight = 35; // Smaller height for suggestions
                int itemSpacing = 1;
                int maxSuggestions = 8; // Show up to 8 suggestions

                for (int i = 0; i < g_autocompleteSuggestions.size() && i < maxSuggestions; ++i) {
                    const std::wstring& suggestion = g_autocompleteSuggestions[i];
                    RECT rcItem = { 12, startY, rcClient.right - 12, startY + itemHeight };

                    // Highlight selected suggestion
                    if (i == g_selectedSuggestion) {
                        DrawRoundedRect(hdcMem, rcItem, 4, COLOR_BG_SELECTED, RGB(90, 90, 90), 1, 1.0f);
                    }

                    // Draw suggestion text
                    RECT rcText = { rcItem.left + 15, rcItem.top + 5, rcItem.right - 15, rcItem.bottom - 5 };
                    SelectObject(hdcMem, g_hFont);
                    
                    // Highlight the matching part
                    if (i == g_selectedSuggestion) {
                        SetTextColor(hdcMem, COLOR_TEXT_PRIMARY);
                    } else {
                        SetTextColor(hdcMem, COLOR_TEXT_SECONDARY);
                    }
                    
                    DrawTextW(hdcMem, suggestion.c_str(), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    
                    startY += itemHeight + itemSpacing;
                }
                
                // Add a subtle hint at the bottom
                if (!g_autocompleteSuggestions.empty()) {
                    startY += 5;
                    RECT rcHint = { 12, startY, rcClient.right - 12, startY + 20 };
                    SelectObject(hdcMem, g_hDescFont);
                    SetTextColor(hdcMem, RGB(120, 120, 120));
                    DrawTextW(hdcMem, L"Tab: Nächster Vorschlag • Escape: Zurück • Enter: Ausführen", -1, &rcHint, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                }
            }
            else if (showNoResults) {
                // Show "No results found" message when user has typed something but no commands match
                int startY = rcInput.bottom + 10;
                int itemHeight = 55;
                
                RECT rcNoResults = { 12, startY, rcClient.right - 12, startY + itemHeight };
                
                // Subtle background
                RECT rcItemBg = rcNoResults;
                rcItemBg.left += 2;
                rcItemBg.top += 1;
                rcItemBg.right -= 2;
                rcItemBg.bottom -= 1;
                DrawRoundedRect(hdcMem, rcItemBg, 4, RGB(50, 50, 50), 0, 0, 0.5f);
                
                // Draw "No results" icon
                RECT rcIcon = { rcNoResults.left + 15, rcNoResults.top + 15, rcNoResults.left + 45, rcNoResults.bottom - 15 };
                SelectObject(hdcMem, g_hIconFont);
                SetTextColor(hdcMem, RGB(120, 120, 120));
                DrawTextW(hdcMem, L"🔍", -1, &rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                // Draw "No results found" text
                RECT rcText = { rcNoResults.left + 55, rcNoResults.top + 8, rcNoResults.right - 15, rcNoResults.bottom - 8 };
                SelectObject(hdcMem, g_hFont);
                SetTextColor(hdcMem, RGB(150, 150, 150));
                DrawTextW(hdcMem, (L"Keine Ergebnisse für \"" + g_inputBuffer + L"\" gefunden").c_str(), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }


            // Copy back buffer to screen
            BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);

            // Clean up
            SelectObject(hdcMem, hOldFont);
            SelectObject(hdcMem, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
        }
        break;
        case WM_CHAR:
            if (wParam > 31) { // Ignore control characters
                g_inputBuffer += static_cast<wchar_t>(wParam);
                UpdateFoundCommands(g_inputBuffer);
                g_cursorVisible = true; // Reset cursor visibility when typing
                g_lastCursorBlink = std::chrono::steady_clock::now();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_KEYDOWN:
            if (wParam == VK_BACK) {
                if (!g_inputBuffer.empty()) {
                    g_inputBuffer.pop_back();
                    UpdateFoundCommands(g_inputBuffer);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                break;
            }
            
            if (wParam == VK_ESCAPE) {
                if (g_isAutocompleteMode && g_selectedSuggestion != -1) {
                    // Erste Escape: Zurück zur ursprünglichen Eingabe
                    g_inputBuffer = g_originalInput;
                    g_selectedSuggestion = -1;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                else {
                    // Zweite Escape oder normaler Modus: Fenster schließen
                    g_isWindowVisible = false;
                    ShowWindow(g_hwnd, SW_HIDE);
                }
                break;
            }

            if (g_foundCommands.empty() && g_inputBuffer.empty()) break;


            switch (wParam) {
                case VK_UP: {
                    if (!g_foundCommands.empty()) {
                        g_selectedCommand = (g_selectedCommand - 1 + static_cast<int>(g_foundCommands.size())) % static_cast<int>(g_foundCommands.size());
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                case VK_DOWN: {
                    if (!g_foundCommands.empty()) {
                        g_selectedCommand = (g_selectedCommand + 1) % static_cast<int>(g_foundCommands.size());
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                case VK_TAB:
                    if (g_isAutocompleteMode && !g_autocompleteSuggestions.empty()) {
                        // Autocomplete-Modus: Navigiere durch Vorschläge
                        if (GetKeyState(VK_SHIFT) & 0x8000) { // Shift+Tab, move up
                            g_selectedSuggestion = (g_selectedSuggestion - 1 + static_cast<int>(g_autocompleteSuggestions.size())) % static_cast<int>(g_autocompleteSuggestions.size());
                        }
                        else { // Tab, move down
                            g_selectedSuggestion = (g_selectedSuggestion + 1) % static_cast<int>(g_autocompleteSuggestions.size());
                        }
                        
                        // Ersetze die Eingabe mit dem ausgewählten Vorschlag
                        g_inputBuffer = g_autocompleteSuggestions[g_selectedSuggestion];
                        
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    else if (!g_foundCommands.empty()) {
                        // Normaler Modus: Navigiere durch Suchergebnisse
                        if (GetKeyState(VK_SHIFT) & 0x8000) { // Shift+Tab, move up
                            g_selectedCommand = (g_selectedCommand - 1 + static_cast<int>(g_foundCommands.size())) % static_cast<int>(g_foundCommands.size());
                        }
                        else { // Tab, move down
                            g_selectedCommand = (g_selectedCommand + 1) % static_cast<int>(g_foundCommands.size());
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case VK_LEFT:
                case VK_RIGHT:
                    // Navigation left/right is not needed anymore in the new layout.
                    break;
                case VK_RETURN: {
                    if (!g_foundCommands.empty() && g_selectedCommand < g_foundCommands.size()) {
                        // Verwende die neue ExecuteCommand Methode mit History-Tracking
                        g_commandManager.ExecuteCommand(g_foundCommands[g_selectedCommand]);
                        g_isWindowVisible = false;
                        ShowWindow(g_hwnd, SW_HIDE);
                        g_inputBuffer.clear();
                        UpdateFoundCommands(L"");
                    } else if (!g_inputBuffer.empty()) {
                        // Prüfe zuerst auf Shebang-Commands (!l, !t etc.)
                        if (g_commandManager.IsShebangCommand(g_inputBuffer)) {
                            if (g_commandManager.ExecuteShebangCommand(g_inputBuffer)) {
                                g_isWindowVisible = false;
                                ShowWindow(g_hwnd, SW_HIDE);
                                g_inputBuffer.clear();
                                UpdateFoundCommands(L"");
                            }
                            // Wenn Shebang-Command fehlschlägt, bleibe im Fenster für weitere Eingabe
                        } 
                        // Prüfe dann auf natürliche Commands (launch, start, terminate etc.)
                        else if (g_commandManager.IsNaturalCommand(g_inputBuffer)) {
                            if (g_commandManager.ExecuteNaturalCommand(g_inputBuffer)) {
                                g_isWindowVisible = false;
                                ShowWindow(g_hwnd, SW_HIDE);
                                g_inputBuffer.clear();
                                UpdateFoundCommands(L"");
                            }
                            // Wenn natürlicher Command fehlschlägt, bleibe im Fenster für weitere Eingabe
                        } 
                        else {
                            // PowerShell fallback mit History-Tracking
                            g_commandManager.ExecutePowerShellCommand(g_inputBuffer);
                            
                            SHELLEXECUTEINFOW sei = { sizeof(sei) };
                            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                            sei.lpVerb = L"open";
                            sei.lpFile = L"powershell.exe";
                            sei.lpParameters = g_inputBuffer.c_str();
                            sei.nShow = SW_SHOW;
                            ShellExecuteExW(&sei);
                            g_isWindowVisible = false;
                            ShowWindow(g_hwnd, SW_HIDE);
                            g_inputBuffer.clear();
                            UpdateFoundCommands(L"");
                        }
                    }
                    break;
                }
            }
            break;

        case WM_HOTKEY:
        {
            int hotkeyId = static_cast<int>(wParam);

            // Handle the main palette hotkey
            if (hotkeyId == HotkeyManager::PALETTE_HOTKEY_ID) {
                g_isWindowVisible = !g_isWindowVisible;
                
                if (g_isWindowVisible) {
                    // Reset cursor state
                    g_cursorVisible = true;
                    g_lastCursorBlink = std::chrono::steady_clock::now();
                    
                    // Position window like PowerToys Run
                    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                    int windowWidth = 750;
                    int windowHeight = 65;

                    int x = (screenWidth - windowWidth) / 2;
                    int y = GetSystemMetrics(SM_CYSCREEN) / 4;
                    
                    SetWindowPos(g_hwnd, HWND_TOPMOST, x, y, windowWidth, windowHeight, SWP_SHOWWINDOW);
                    SetForegroundWindow(g_hwnd);
                    SetActiveWindow(g_hwnd);
                    SetFocus(g_hwnd);
                    
                    g_inputBuffer.clear();
                    UpdateFoundCommands(L"");
                    InvalidateRect(g_hwnd, NULL, FALSE);
                } else {
                    ShowWindow(g_hwnd, SW_HIDE);
                }
            }
            // Handle custom command hotkeys
            else {
                std::wstring commandToExecute = g_hotkeyManager.FindCommandForHotkey(hotkeyId);
                if (!commandToExecute.empty()) {
                    // Use a simple natural command execution for now.
                    // This assumes commands like "notepad", "calculator" can be found.
                    std::wstring naturalCommand = L"launch " + commandToExecute;
                    g_commandManager.ExecuteNaturalCommand(naturalCommand);
                }
            }
            break;
        }
        case WM_ACTIVATE:
            // Redraw to show/hide focus glow
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            g_hotkeyManager.UnregisterHotkeys(hwnd);
            if (g_hFont) DeleteObject(g_hFont);
            if (g_hDescFont) DeleteObject(g_hDescFont);
            if (g_hIconFont) DeleteObject(g_hIconFont);
            GdiplusShutdown(g_gdiplusToken);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void RegisterCommands(CommandManager& commandManager)
{
    // Register all plugins using the new system
    commandManager.RegisterAllPlugins();
    
    // Register legacy ProcessTools commands (these will be moved to the new system in future)
    commandManager.RegisterCommand(std::make_unique<EnterProcessModeCommand>());
    commandManager.RegisterCommand(std::make_unique<OpenProcessPathCommand>());
    commandManager.RegisterCommand(std::make_unique<TerminateProcessCommand>());
}

// Entry point for Unicode
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"CommandPaletteWindow";
    const wchar_t MUTEX_NAME[] = L"WinPalCommandPaletteMutex";

    HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        // Another instance is already running
        if (hMutex) {
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
        }
        return 0; // Exit silently
    }

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW; // Redraw on resize

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    g_hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST, // Removed layered attributes for DWM handling
        CLASS_NAME,
        L"WinPal",
        WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 750, 65,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (g_hwnd == NULL) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Use DWM to set modern rounded corners. This is the preferred way on Win11+.
#ifdef DWMWA_WINDOW_CORNER_PREFERENCE
    DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
    DwmSetWindowAttribute(g_hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));
#endif

    RegisterCommands(g_commandManager);
    UpdateFoundCommands(L"");

    if (!g_hotkeyManager.RegisterHotkeys(g_hwnd)) {
        MessageBoxW(NULL, L"Hotkey Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_hotkeyManager.UnregisterHotkeys(g_hwnd);

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return (int)msg.wParam;
}
