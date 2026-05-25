#include "WindowManager.h"
#include "framework.h"

static HWND g_GameWindow = NULL;

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);

    if (pid != GetCurrentProcessId())
        return TRUE;

    if (!IsWindowVisible(hwnd))
        return TRUE;

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));

    if (strlen(title) <= 0)
        return TRUE;

    g_GameWindow = hwnd;
    return FALSE;
}

static HWND FindGameWindow()
{
    g_GameWindow = NULL;
    EnumWindows(EnumWindowsProc, 0);
    return g_GameWindow;
}

bool WindowManager_EnableBorderless()
{
    HWND hwnd = FindGameWindow();

    if (!hwnd)
    {
        Framework_Log("[WINDOW] Game window not found");
        return false;
    }

    LONG style = GetWindowLongA(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);

    style &= ~WS_CAPTION;
    style &= ~WS_THICKFRAME;
    style &= ~WS_MINIMIZEBOX;
    style &= ~WS_MAXIMIZEBOX;
    style &= ~WS_SYSMENU;

    exStyle &= ~WS_EX_DLGMODALFRAME;
    exStyle &= ~WS_EX_CLIENTEDGE;
    exStyle &= ~WS_EX_STATICEDGE;

    SetWindowLongA(hwnd, GWL_STYLE, style);
    SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle);

    SetWindowPos(
        hwnd,
        HWND_TOP,
        0,
        0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );

    Framework_Log("[WINDOW] Borderless fullscreen applied");
    return true;
}

void WindowManager_Init()
{
    Framework_Log("[WINDOW] WindowManager initialized");
    Framework_Log("[WINDOW] F11 = borderless fullscreen");
}

void WindowManager_Update()
{
    // Do not auto-apply. Auto borderless was causing crash.
}