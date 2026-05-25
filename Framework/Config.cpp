#include "Config.h"
#include "framework.h"

#include <windows.h>
#include <string>
#include <stdio.h>

static char g_ConfigPath[MAX_PATH];

static void WriteDefault(const char* section, const char* key, const char* value)
{
    char current[64];

    GetPrivateProfileStringA(
        section,
        key,
        "",
        current,
        sizeof(current),
        g_ConfigPath
    );

    if (strlen(current) == 0)
        WritePrivateProfileStringA(section, key, value, g_ConfigPath);
}

void Config_Init()
{
    std::wstring path = Framework_GetFolder() + L"\\config.ini";

    WideCharToMultiByte(
        CP_ACP,
        0,
        path.c_str(),
        -1,
        g_ConfigPath,
        MAX_PATH,
        NULL,
        NULL
    );

    WriteDefault("Overlay", "ShowHud", "1");
    WriteDefault("Overlay", "MenuX", "40");
    WriteDefault("Overlay", "MenuY", "80");

    WriteDefault("Window", "NoBorder", "0");

    WriteDefault("Mods", "Enabled", "1");

    WriteDefault("Patches", "SkipIntro", "0");

    Framework_Log("[CONFIG] config.ini initialized");
}

bool Config_GetBool(const char* section, const char* key, bool fallback)
{
    return GetPrivateProfileIntA(
        section,
        key,
        fallback ? 1 : 0,
        g_ConfigPath
    ) != 0;
}

int Config_GetInt(const char* section, const char* key, int fallback)
{
    return GetPrivateProfileIntA(
        section,
        key,
        fallback,
        g_ConfigPath
    );
}

void Config_SetBool(const char* section, const char* key, bool value)
{
    WritePrivateProfileStringA(
        section,
        key,
        value ? "1" : "0",
        g_ConfigPath
    );
}

void Config_SetInt(const char* section, const char* key, int value)
{
    char buffer[32];

    sprintf_s(
        buffer,
        sizeof(buffer),
        "%d",
        value
    );

    WritePrivateProfileStringA(
        section,
        key,
        buffer,
        g_ConfigPath
    );
}