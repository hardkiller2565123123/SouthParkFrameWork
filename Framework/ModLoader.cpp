#include "ModLoader.h"
#include "framework.h"
#include "Config.h"
#include "Notification.h"

#include <windows.h>
#include <vector>
#include <string>

typedef void(__cdecl* SPFW_OnLoad_t)();
typedef void(__cdecl* SPFW_OnUnload_t)();

struct ModEntry
{
    std::wstring NameW;
    std::string NameA;
    std::wstring Path;
    bool Enabled;
    HMODULE Module;
    SPFW_OnUnload_t OnUnload;
};

static std::vector<ModEntry> g_Mods;

static std::wstring ModsFolder()
{
    return Framework_GetFolder() + L"\\Mods";
}

static std::string WideToAnsi(const std::wstring& text)
{
    char buffer[MAX_PATH];

    WideCharToMultiByte(
        CP_ACP,
        0,
        text.c_str(),
        -1,
        buffer,
        MAX_PATH,
        NULL,
        NULL
    );

    return buffer;
}

static bool IsCoreDll(const wchar_t* name)
{
    if (!name)
        return true;

    return
        _wcsicmp(name, L"Framework.dll") == 0 ||
        _wcsicmp(name, L"Logger.dll") == 0 ||
        _wcsicmp(name, L"SteamOnline.dll") == 0 ||
        _wcsicmp(name, L"steam_api.dll") == 0 ||
        _wcsicmp(name, L"D3DX9_43.dll") == 0 ||
        _wcsicmp(name, L"MinHook.x86.dll") == 0;
}

static std::string ConfigSectionForMod(const std::string& name)
{
    return "Mod_" + name;
}

static void DetectDllMods()
{
    std::wstring search = ModsFolder() + L"\\*.dll";

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(search.c_str(), &data);

    if (find == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (IsCoreDll(data.cFileName))
            continue;

        ModEntry mod;
        mod.NameW = data.cFileName;
        mod.NameA = WideToAnsi(mod.NameW);
        mod.Path = ModsFolder() + L"\\" + data.cFileName;
        mod.Module = NULL;
        mod.OnUnload = NULL;

        std::string section = ConfigSectionForMod(mod.NameA);
        mod.Enabled = Config_GetBool(section.c_str(), "Enabled", true);

        g_Mods.push_back(mod);

    } while (FindNextFileW(find, &data));

    FindClose(find);
}

static void LoadModDll(ModEntry& mod)
{
    if (!mod.Enabled)
        return;

    HMODULE module = LoadLibraryW(mod.Path.c_str());

    if (!module)
    {
        Notification_Push("[MODS] Failed to load DLL mod");
        return;
    }

    SPFW_OnLoad_t onLoad =
        (SPFW_OnLoad_t)GetProcAddress(module, "SPFW_OnLoad");

    SPFW_OnUnload_t onUnload =
        (SPFW_OnUnload_t)GetProcAddress(module, "SPFW_OnUnload");

    if (onLoad)
        onLoad();

    mod.Module = module;
    mod.OnUnload = onUnload;

    Notification_Push("[MODS] DLL mod loaded");
}

void ModLoader_Init()
{
    CreateDirectoryW(ModsFolder().c_str(), NULL);

    Framework_Log("[MODS] ModLoader initialized");
}

void ModLoader_LoadAll()
{
    ModLoader_UnloadAll();

    g_Mods.clear();

    DetectDllMods();

    for (size_t i = 0; i < g_Mods.size(); i++)
        LoadModDll(g_Mods[i]);

    Notification_Push("[MODS] DLL mods loaded");
}

void ModLoader_UnloadAll()
{
    for (size_t i = 0; i < g_Mods.size(); i++)
    {
        if (g_Mods[i].OnUnload)
            g_Mods[i].OnUnload();

        if (g_Mods[i].Module)
            FreeLibrary(g_Mods[i].Module);

        g_Mods[i].Module = NULL;
        g_Mods[i].OnUnload = NULL;
    }
}

void ModLoader_Reload()
{
    Notification_Push("[MODS] Reloading DLL mods");
    ModLoader_LoadAll();
}

int ModLoader_GetLoadedCount()
{
    int count = 0;

    for (size_t i = 0; i < g_Mods.size(); i++)
    {
        if (g_Mods[i].Enabled && g_Mods[i].Module)
            count++;
    }

    return count;
}

int ModLoader_GetDetectedCount()
{
    return (int)g_Mods.size();
}

const char* ModLoader_GetModName(int index)
{
    if (index < 0 || index >= (int)g_Mods.size())
        return "";

    return g_Mods[index].NameA.c_str();
}

bool ModLoader_IsModEnabled(int index)
{
    if (index < 0 || index >= (int)g_Mods.size())
        return false;

    return g_Mods[index].Enabled;
}

void ModLoader_ToggleMod(int index)
{
    if (index < 0 || index >= (int)g_Mods.size())
        return;

    g_Mods[index].Enabled = !g_Mods[index].Enabled;

    std::string section = ConfigSectionForMod(g_Mods[index].NameA);

    Config_SetBool(
        section.c_str(),
        "Enabled",
        g_Mods[index].Enabled
    );

    Notification_Push(
        g_Mods[index].Enabled
        ? "[MODS] DLL mod enabled"
        : "[MODS] DLL mod disabled"
    );

    ModLoader_Reload();
}