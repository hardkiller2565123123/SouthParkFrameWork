#include "framework.h"
#include "Input.h"
#include "PatchManager.h"
#include "HookManager.h"
#include "WindowManager.h"
#include "Overlay.h"
#include "MinHook.h"
#include "Config.h"
#include "ModLoader.h"
#include "Notification.h"
#include "PatchToggles.h"
#include "PatchFileSystem.h"
#include "VirtualFileSystem.h"


static HMODULE g_Logger = NULL;

typedef void(__cdecl* Logger_Log_t)(const char*);
static Logger_Log_t g_Log = NULL;

void Framework_Log(const char* text)
{
    if (g_Log && text)
        g_Log(text);
}

std::wstring Framework_GetExeFolder()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    std::wstring p = path;
    size_t slash = p.find_last_of(L"\\/");

    if (slash != std::wstring::npos)
        p = p.substr(0, slash);

    return p;
}

std::wstring Framework_GetFolder()
{
    return Framework_GetExeFolder() + L"\\SouthParkFrameWork";
}

static void LoadLogger()
{
    std::wstring path = Framework_GetFolder() + L"\\Logger.dll";

    g_Logger = LoadLibraryW(path.c_str());

    if (!g_Logger)
        return;

    g_Log = (Logger_Log_t)GetProcAddress(g_Logger, "Logger_Log");
}

static void LogDiscoveredEngineSystems()
{
    Framework_Log("[SCAN] D3D9 renderer strings detected");
    Framework_Log("[SCAN] Scaleform D3D9 HAL strings detected");
    Framework_Log("[SCAN] DrawText Manager strings detected");
    Framework_Log("[SCAN] Debug text command strings detected");
    Framework_Log("[SCAN] Texture and D3DX loader strings detected");
}

DWORD WINAPI Framework_MainThread(LPVOID)
{
    LoadLogger();

    Framework_Log("[FRAMEWORK] Framework.dll initialized");

    if (MH_Initialize() != MH_OK)
    {
        Framework_Log("[FRAMEWORK] MinHook failed");
        return 0;
    }

    Framework_Log("[FRAMEWORK] MinHook initialized");

    LogDiscoveredEngineSystems();

    Overlay_Init();
    PatchManager_Init();
    HookManager_Init();
    Input_Init();
    WindowManager_Init();
    Config_Init();
    Notification_Init();
    PatchToggles_Init();
    ModLoader_Init();
    PatchFileSystem_Init();
    VFS_Init();

    if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
        Framework_Log("[FRAMEWORK] Hooks enabled");
    else
        Framework_Log("[FRAMEWORK] Hook enable failed");

    Framework_Log("[FRAMEWORK] Runtime systems initialized");

    if (Config_GetBool("Mods", "Enabled", true))
        ModLoader_LoadAll();

    while (true)
    {
        Input_Update();
        WindowManager_Update();

        Sleep(16);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        CreateThread(
            NULL,
            0,
            Framework_MainThread,
            NULL,
            0,
            NULL
        );
    }

    if (reason == DLL_PROCESS_DETACH)
    {   
        ModLoader_UnloadAll();
        Overlay_Shutdown();

        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }

    return TRUE;
}