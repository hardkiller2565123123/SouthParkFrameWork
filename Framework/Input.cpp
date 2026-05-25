#include "Input.h"
#include "framework.h"
#include "Overlay.h"
#include "WindowManager.h"

#include <windows.h>

static bool g_PreviousKeys[256];

void Input_Init()
{
    ZeroMemory(g_PreviousKeys, sizeof(g_PreviousKeys));

    Framework_Log("[INPUT] Input system initialized");
    Framework_Log("[INPUT] F1 = overlay toggle");
    Framework_Log("[INPUT] F11 = borderless fullscreen");
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
    {
        Overlay_Toggle();
    }

    if (Input_IsKeyPressed(VK_F11))
    {
        WindowManager_EnableBorderless();
    }
}