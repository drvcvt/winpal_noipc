#pragma once

#include <windows.h>
#include <string>

class GuiManager {
public:
    GuiManager();
    ~GuiManager();

    void CreateComponents(HWND hWndParent, HINSTANCE hInstance);

private:
    // No members needed for immediate mode GUI yet
};
