#include "ClearClipboardCommand.h"
#include <windows.h>

std::wstring ClearClipboardCommand::GetName() const {
    return L"Clear Clipboard";
}

std::wstring ClearClipboardCommand::GetDescription() const {
    return L"Clears the current contents of the Windows clipboard.";
}

CommandCategory ClearClipboardCommand::GetCategory() const {
    return CommandCategory::CLIPBOARD_MANAGER;
}

void ClearClipboardCommand::Execute() {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        CloseClipboard();
        MessageBoxW(NULL, L"Clipboard cleared successfully!", L"Info", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(NULL, L"Failed to access clipboard!", L"Error", MB_OK | MB_ICONERROR);
    }
} 