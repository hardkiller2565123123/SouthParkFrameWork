#include "Overlay.h"
#include "framework.h"
#include "WindowManager.h"
#include "Config.h"
#include "ModLoader.h"
#include "PatchToggles.h"
#include "Notification.h"
#include "PatchFileSystem.h"

#include <windows.h>
#include <d3dx9.h>
#include <stdio.h>
#include <string.h>

static ID3DXFont* g_Font = NULL;
static ID3DXLine* g_Line = NULL;

static bool g_ShowHud = true;
static bool g_ShowMenu = false;
static bool g_NoBorderEnabled = false;
static bool g_DeviceLost = false;
static bool g_CursorForced = false;

static bool g_LoggedFontCreated = false;
static bool g_LoggedFontFailed = false;

static int g_MenuIndex = 0;
static int g_MenuX = 40;
static int g_MenuY = 80;
static int g_MenuW = 430;
static int g_MenuH = 260;

static bool g_Dragging = false;
static int g_DragOffsetX = 0;
static int g_DragOffsetY = 0;
static bool g_LastMouseDown = false;

static const char* SPFW_VERSION = "1.00";

enum MenuPage
{
    PAGE_MAIN = 0,
    PAGE_PATCHES,
    PAGE_MODS,
    PAGE_ABOUT
};

enum MenuAction
{
    ACTION_TOGGLE_HUD = 0,
    ACTION_TOGGLE_NO_BORDER,
    ACTION_OPEN_PATCHES,
    ACTION_OPEN_MODS,
    ACTION_OPEN_ABOUT,
    ACTION_RELOAD_MODS,
    ACTION_RELOAD_FILE_PATCHES,
    ACTION_BACK_MAIN,
    ACTION_NONE
};

struct MenuEntry
{
    MenuAction Action;
    const char* Label;
    const char* Value;
};

static MenuPage g_Page = PAGE_MAIN;

static int GetMenuCount()
{
    if (g_Page == PAGE_MAIN)
        return 5;

    if (g_Page == PAGE_PATCHES)
        return PatchFileSystem_GetPatchCount() + 2;

    if (g_Page == PAGE_MODS)
        return 3;

    if (g_Page == PAGE_ABOUT)
        return 1;

    return 0;
}

static MenuEntry GetEntry(int index)
{
    if (g_Page == PAGE_MAIN)
    {
        static MenuEntry entries[] =
        {
            { ACTION_TOGGLE_HUD,       "Show HUD",  NULL },
            { ACTION_TOGGLE_NO_BORDER, "No Border", NULL },
            { ACTION_OPEN_PATCHES,     "Patches",   NULL },
            { ACTION_OPEN_MODS,        "Mods",      NULL },
            { ACTION_OPEN_ABOUT,       "About",     NULL }
        };

        return entries[index];
    }

    if (g_Page == PAGE_PATCHES)
    {
        int patchCount = PatchFileSystem_GetPatchCount();

        if (index < patchCount)
        {
            static MenuEntry patchEntry;

            patchEntry.Action = ACTION_NONE;
            patchEntry.Label = PatchFileSystem_GetPatchName(index);
            patchEntry.Value = PatchFileSystem_IsPatchEnabled(index) ? "[x]" : "[ ]";

            return patchEntry;
        }

        if (index == patchCount)
        {
            static MenuEntry reloadEntry =
            {
                ACTION_RELOAD_FILE_PATCHES,
                "Refresh File Patches",
                NULL
            };

            return reloadEntry;
        }

        static MenuEntry backEntry =
        {
            ACTION_BACK_MAIN,
            "Back",
            NULL
        };

        return backEntry;
    }

    if (g_Page == PAGE_MODS)
    {
        static char modCountText[32];

        sprintf_s(
            modCountText,
            sizeof(modCountText),
            "%d",
            ModLoader_GetLoadedCount()
        );

        static MenuEntry entries[3];

        entries[0].Action = ACTION_NONE;
        entries[0].Label = "Loaded DLL Mods";
        entries[0].Value = modCountText;

        entries[1].Action = ACTION_RELOAD_MODS;
        entries[1].Label = "Reload DLL Mods";
        entries[1].Value = NULL;

        entries[2].Action = ACTION_BACK_MAIN;
        entries[2].Label = "Back";
        entries[2].Value = NULL;

        return entries[index];
    }

    static MenuEntry about[] =
    {
        { ACTION_BACK_MAIN, "Back", NULL }
    };

    return about[0];
}

void Overlay_Init()
{
    g_ShowHud = Config_GetBool("Overlay", "ShowHud", true);
    g_NoBorderEnabled = Config_GetBool("Window", "NoBorder", false);
    g_MenuX = Config_GetInt("Overlay", "MenuX", 40);
    g_MenuY = Config_GetInt("Overlay", "MenuY", 80);

    Framework_Log("[OVERLAY] Overlay initialized");
}

void Overlay_Shutdown()
{
    Config_SetInt("Overlay", "MenuX", g_MenuX);
    Config_SetInt("Overlay", "MenuY", g_MenuY);
    Config_SetBool("Overlay", "ShowHud", g_ShowHud);
    Config_SetBool("Window", "NoBorder", g_NoBorderEnabled);

    if (g_Line)
    {
        g_Line->Release();
        g_Line = NULL;
    }

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

    if (g_Line)
        g_Line->OnLostDevice();
}

void Overlay_OnResetDevice()
{
    if (g_Font)
        g_Font->OnResetDevice();

    if (g_Line)
        g_Line->OnResetDevice();

    g_DeviceLost = false;
}

bool Overlay_IsMenuOpen()
{
    return g_ShowMenu;
}

void Overlay_ToggleHud()
{
    g_ShowHud = !g_ShowHud;
    Config_SetBool("Overlay", "ShowHud", g_ShowHud);

    Notification_Push(
        g_ShowHud
        ? "[OVERLAY] HUD enabled"
        : "[OVERLAY] HUD disabled"
    );
}

void Overlay_ToggleMenu()
{
    g_ShowMenu = !g_ShowMenu;

    Notification_Push(
        g_ShowMenu
        ? "[OVERLAY] F1 menu opened"
        : "[OVERLAY] F1 menu closed"
    );
}

void Overlay_MenuUp()
{
    if (!g_ShowMenu)
        return;

    g_MenuIndex--;

    if (g_MenuIndex < 0)
        g_MenuIndex = GetMenuCount() - 1;
}

void Overlay_MenuDown()
{
    if (!g_ShowMenu)
        return;

    g_MenuIndex++;

    if (g_MenuIndex >= GetMenuCount())
        g_MenuIndex = 0;
}

void Overlay_MenuSelect()
{
    if (!g_ShowMenu)
        return;

    MenuEntry entry = GetEntry(g_MenuIndex);

    if (g_Page == PAGE_PATCHES && entry.Action == ACTION_NONE)
    {
        PatchFileSystem_TogglePatch(g_MenuIndex);
        return;
    }

    if (entry.Action == ACTION_TOGGLE_HUD)
    {
        Overlay_ToggleHud();
    }
    else if (entry.Action == ACTION_TOGGLE_NO_BORDER)
    {
        g_NoBorderEnabled = !g_NoBorderEnabled;

        if (g_NoBorderEnabled)
        {
            WindowManager_EnableNoBorder();
            Notification_Push("[WINDOW] No border enabled");
        }
        else
        {
            WindowManager_RestoreWindow();
            Notification_Push("[WINDOW] No border disabled");
        }

        Config_SetBool("Window", "NoBorder", g_NoBorderEnabled);
    }
    else if (entry.Action == ACTION_OPEN_PATCHES)
    {
        PatchFileSystem_Reload();
        g_Page = PAGE_PATCHES;
        g_MenuIndex = 0;
    }
    else if (entry.Action == ACTION_OPEN_MODS)
    {
        g_Page = PAGE_MODS;
        g_MenuIndex = 0;
    }
    else if (entry.Action == ACTION_OPEN_ABOUT)
    {
        g_Page = PAGE_ABOUT;
        g_MenuIndex = 0;
    }
    else if (entry.Action == ACTION_RELOAD_MODS)
    {
        ModLoader_Reload();
    }
    else if (entry.Action == ACTION_RELOAD_FILE_PATCHES)
    {
        PatchFileSystem_Reload();
        g_MenuIndex = 0;
    }
    else if (entry.Action == ACTION_BACK_MAIN)
    {
        g_Page = PAGE_MAIN;
        g_MenuIndex = 0;
    }
}

static const char* GetDynamicValue(MenuAction action)
{
    if (action == ACTION_TOGGLE_HUD)
        return g_ShowHud ? "[x]" : "[ ]";

    if (action == ACTION_TOGGLE_NO_BORDER)
        return g_NoBorderEnabled ? "[x]" : "[ ]";

    return NULL;
}

static const char* GetPageName()
{
    if (g_Page == PAGE_PATCHES)
        return "Patches";

    if (g_Page == PAGE_MODS)
        return "Mods";

    if (g_Page == PAGE_ABOUT)
        return "About";

    return "Main";
}

static void UpdateCursor()
{
    if (g_ShowMenu)
    {
        if (!g_CursorForced)
        {
            while (ShowCursor(TRUE) < 0) {}
            g_CursorForced = true;
        }
    }
    else
    {
        if (g_CursorForced)
        {
            while (ShowCursor(FALSE) >= 0) {}
            g_CursorForced = false;
        }
    }
}

static void CreateResourcesIfNeeded(LPDIRECT3DDEVICE9 device)
{
    if (!device)
        return;

    if (!g_Font)
    {
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

    if (!g_Line)
        D3DXCreateLine(device, &g_Line);
}

static void DrawTextSimple(
    const char* text,
    int x,
    int y,
    D3DCOLOR color
)
{
    if (!g_Font || !text)
        return;

    RECT shadow;
    shadow.left = x + 1;
    shadow.top = y + 1;
    shadow.right = x + 900;
    shadow.bottom = y + 400;

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + 900;
    rect.bottom = y + 400;

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

static void DrawFilledRect(
    LPDIRECT3DDEVICE9 device,
    int x,
    int y,
    int w,
    int h,
    D3DCOLOR color
)
{
    if (!device)
        return;

    D3DRECT r;
    r.x1 = x;
    r.y1 = y;
    r.x2 = x + w;
    r.y2 = y + h;

    device->Clear(
        1,
        &r,
        D3DCLEAR_TARGET,
        color,
        0.0f,
        0
    );
}

static void DrawBorder(
    int x,
    int y,
    int w,
    int h,
    D3DCOLOR color
)
{
    if (!g_Line)
        return;

    D3DXVECTOR2 lines[5];

    lines[0] = D3DXVECTOR2((float)x, (float)y);
    lines[1] = D3DXVECTOR2((float)(x + w), (float)y);
    lines[2] = D3DXVECTOR2((float)(x + w), (float)(y + h));
    lines[3] = D3DXVECTOR2((float)x, (float)(y + h));
    lines[4] = D3DXVECTOR2((float)x, (float)y);

    g_Line->SetWidth(1.0f);
    g_Line->Draw(lines, 5, color);
}

static bool IsMouseInside(int mx, int my, int x, int y, int w, int h)
{
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

static bool GetMouseClientPos(LPDIRECT3DDEVICE9 device, POINT* out)
{
    if (!device || !out)
        return false;

    D3DDEVICE_CREATION_PARAMETERS params;

    if (FAILED(device->GetCreationParameters(&params)))
        return false;

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(params.hFocusWindow, &pt);

    out->x = pt.x;
    out->y = pt.y;

    return true;
}

static void UpdateMouseMenu(LPDIRECT3DDEVICE9 device)
{
    if (!g_ShowMenu)
    {
        g_Dragging = false;
        g_LastMouseDown = false;
        return;
    }

    POINT mouse;

    if (!GetMouseClientPos(device, &mouse))
        return;

    bool mouseDown =
        (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    bool mouseClicked =
        mouseDown && !g_LastMouseDown;

    if (
        mouseClicked &&
        IsMouseInside(mouse.x, mouse.y, g_MenuX, g_MenuY, g_MenuW, 34)
        )
    {
        g_Dragging = true;
        g_DragOffsetX = mouse.x - g_MenuX;
        g_DragOffsetY = mouse.y - g_MenuY;
    }

    if (!mouseDown)
        g_Dragging = false;

    if (g_Dragging)
    {
        g_MenuX = mouse.x - g_DragOffsetX;
        g_MenuY = mouse.y - g_DragOffsetY;
    }

    int itemX = g_MenuX + 20;
    int itemY = g_MenuY + 55;
    int itemH = 22;
    int itemW = g_MenuW - 40;

    for (int i = 0; i < GetMenuCount(); i++)
    {
        int y = itemY + (i * 24);

        if (IsMouseInside(mouse.x, mouse.y, itemX, y, itemW, itemH))
        {
            g_MenuIndex = i;

            if (mouseClicked)
                Overlay_MenuSelect();
        }
    }

    g_LastMouseDown = mouseDown;
}

static void DrawHud()
{
    char text[256];

    sprintf_s(
        text,
        sizeof(text),
        "SouthParkFrameWork v%s\n"
        "Build: %s %s\n"
        "F1: Menu",
        SPFW_VERSION,
        __DATE__,
        __TIME__
    );

    DrawTextSimple(
        text,
        12,
        12,
        D3DCOLOR_ARGB(235, 220, 220, 220)
    );
}

static void DrawMenuItem(int index, int x, int y)
{
    MenuEntry entry = GetEntry(index);

    const char* value = entry.Value;
    const char* dynamicValue = GetDynamicValue(entry.Action);

    if (dynamicValue)
        value = dynamicValue;

    bool selected = g_MenuIndex == index;

    char line[256];

    if (value && value[0] == '[')
    {
        sprintf_s(
            line,
            sizeof(line),
            "%s  %s %s",
            selected ? ">" : " ",
            value,
            entry.Label
        );
    }
    else if (value)
    {
        sprintf_s(
            line,
            sizeof(line),
            "%s  %s: %s",
            selected ? ">" : " ",
            entry.Label,
            value
        );
    }
    else
    {
        sprintf_s(
            line,
            sizeof(line),
            "%s  %s",
            selected ? ">" : " ",
            entry.Label
        );
    }

    DrawTextSimple(
        line,
        x,
        y,
        selected
        ? D3DCOLOR_ARGB(255, 80, 255, 80)
        : D3DCOLOR_ARGB(235, 220, 220, 220)
    );
}

static int CalculateMenuHeight()
{
    int header = 34;
    int topPadding = 22;
    int itemArea = GetMenuCount() * 24;
    int footer = 48;

    return header + topPadding + itemArea + footer;
}

static void DrawAboutPage()
{
    DrawTextSimple(
        "SouthParkFrameWork\n"
        "Version: 1.00\n"
        "Renderer: Direct3D9\n"
        "Core: steam_api proxy + Framework.dll\n"
        "DLL Mods: SouthParkFrameWork\\Mods\n"
        "File Patches: SouthParkFrameWork\\Patches",
        g_MenuX + 20,
        g_MenuY + 55,
        D3DCOLOR_ARGB(235, 220, 220, 220)
    );
}

static void DrawMenu(LPDIRECT3DDEVICE9 device)
{
    g_MenuH = CalculateMenuHeight();

    if (g_Page == PAGE_ABOUT)
        g_MenuH += 110;

    DrawFilledRect(
        device,
        g_MenuX,
        g_MenuY,
        g_MenuW,
        g_MenuH,
        D3DCOLOR_ARGB(175, 10, 10, 10)
    );

    DrawFilledRect(
        device,
        g_MenuX,
        g_MenuY,
        g_MenuW,
        34,
        D3DCOLOR_ARGB(210, 25, 25, 25)
    );

    DrawBorder(
        g_MenuX,
        g_MenuY,
        g_MenuW,
        g_MenuH,
        D3DCOLOR_ARGB(255, 80, 255, 80)
    );

    char title[128];

    sprintf_s(
        title,
        sizeof(title),
        "SouthParkFrameWork - %s",
        GetPageName()
    );

    DrawTextSimple(
        title,
        g_MenuX + 12,
        g_MenuY + 9,
        D3DCOLOR_ARGB(255, 80, 255, 80)
    );

    DrawTextSimple(
        "v1.00",
        g_MenuX + g_MenuW - 55,
        g_MenuY + 9,
        D3DCOLOR_ARGB(235, 220, 220, 220)
    );

    if (g_Page == PAGE_ABOUT)
        DrawAboutPage();

    int itemX = g_MenuX + 20;
    int itemY = g_MenuY + 55;

    if (g_Page == PAGE_ABOUT)
        itemY += 125;

    for (int i = 0; i < GetMenuCount(); i++)
        DrawMenuItem(i, itemX, itemY + (i * 24));

    DrawTextSimple(
        "Drag title bar | Click items | UP/DOWN ENTER | F1 Close",
        g_MenuX + 20,
        g_MenuY + g_MenuH - 35,
        D3DCOLOR_ARGB(210, 180, 180, 180)
    );
}

void Overlay_Render(LPDIRECT3DDEVICE9 device)
{
    if (!device || g_DeviceLost)
        return;

    CreateResourcesIfNeeded(device);
    UpdateCursor();

    if (!g_Font)
        return;

    UpdateMouseMenu(device);

    if (g_ShowHud)
        DrawHud();

    if (g_ShowMenu)
        DrawMenu(device);

    Notification_Render(device, g_Font);
}