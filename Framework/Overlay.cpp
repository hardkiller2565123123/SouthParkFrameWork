#include "Overlay.h"
#include "framework.h"

#include <windows.h>
#include <d3dx9.h>
#include <stdio.h>

static ID3DXFont* g_Font = NULL;

static bool g_ShowOverlay = true;
static bool g_DeviceLost = false;
static bool g_LoggedFontCreated = false;
static bool g_LoggedFontFailed = false;

static const char* SPFW_VERSION = "1.00";

void Overlay_Init()
{
    Framework_Log("[OVERLAY] Overlay initialized");
}

void Overlay_Shutdown()
{
    if (g_Font)
    {
        g_Font->Release();
        g_Font = NULL;
    }

    Framework_Log("[OVERLAY] Overlay shutdown");
}

void Overlay_OnLostDevice()
{
    g_DeviceLost = true;

    if (g_Font)
        g_Font->OnLostDevice();
}

void Overlay_OnResetDevice()
{
    if (g_Font)
        g_Font->OnResetDevice();

    g_DeviceLost = false;
}

void Overlay_Toggle()
{
    g_ShowOverlay = !g_ShowOverlay;

    if (g_ShowOverlay)
        Framework_Log("[OVERLAY] Overlay enabled");
    else
        Framework_Log("[OVERLAY] Overlay disabled");
}

static void CreateFontIfNeeded(LPDIRECT3DDEVICE9 device)
{
    if (!device || g_Font)
        return;

    HRESULT hr = D3DXCreateFontA(
        device,
        15,
        0,
        FW_BOLD,
        1,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Consolas",
        &g_Font
    );

    if (FAILED(hr) || !g_Font)
    {
        if (!g_LoggedFontFailed)
        {
            Framework_Log("[OVERLAY] D3DXCreateFontA failed");
            g_LoggedFontFailed = true;
        }

        return;
    }

    if (!g_LoggedFontCreated)
    {
        Framework_Log("[OVERLAY] D3D9 font created");
        g_LoggedFontCreated = true;
    }
}

static void DrawTextBlock(
    LPDIRECT3DDEVICE9 device,
    const char* text,
    int x,
    int y,
    D3DCOLOR color
)
{
    if (!device || !g_Font || !text)
        return;

    RECT shadow;
    shadow.left = x + 1;
    shadow.top = y + 1;
    shadow.right = x + 600 + 1;
    shadow.bottom = y + 180 + 1;

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + 600;
    rect.bottom = y + 180;

    g_Font->DrawTextA(
        NULL,
        text,
        -1,
        &shadow,
        DT_LEFT | DT_NOCLIP,
        D3DCOLOR_ARGB(220, 0, 0, 0)
    );

    g_Font->DrawTextA(
        NULL,
        text,
        -1,
        &rect,
        DT_LEFT | DT_NOCLIP,
        color
    );
}

void Overlay_Render(LPDIRECT3DDEVICE9 device)
{
    if (!g_ShowOverlay || !device || g_DeviceLost)
        return;

    CreateFontIfNeeded(device);

    if (!g_Font)
        return;

    char text[512];

    sprintf_s(
        text,
        sizeof(text),
        "SouthParkFrameWork v%s\n"
        "Build: %s %s\n"
        "DLL: Framework.dll\n"
        "DLL: Logger.dll\n"
        "DLL: steam_api.dll\n"
        "Status: Initialized",
        SPFW_VERSION,
        __DATE__,
        __TIME__
    );

    DrawTextBlock(
        device,
        text,
        12,
        12,
        D3DCOLOR_ARGB(235, 220, 220, 220)
    );
}