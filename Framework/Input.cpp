#include "Input.h"
#include "framework.h"
#include "Overlay.h"

#include <windows.h>

static bool g_PreviousKeys[256];

void Input_Init()
{
    ZeroMemory(g_PreviousKeys, sizeof(g_PreviousKeys));

    Framework_Log("[INPUT] Input system initialized");
    Framework_Log("[INPUT] F1 = overlay menu");
}

bool Input_IsKeyDown(int vk)
{
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool Input_IsKeyPressed(int vk)
{
    bool down = Input_IsKeyDown(vk);
    bool pressed = down && !g_PreviousKeys[vk];

    g_PreviousKeys[vk] = down;

    return pressed;
}

void Input_Update()
{
    if (Input_IsKeyPressed(VK_F1))
        Overlay_ToggleMenu();

    if (Overlay_IsMenuOpen())
    {
        if (Input_IsKeyPressed(VK_UP))
            Overlay_MenuUp();

        if (Input_IsKeyPressed(VK_DOWN))
            Overlay_MenuDown();

        if (Input_IsKeyPressed(VK_RETURN))
            Overlay_MenuSelect();
    }
}