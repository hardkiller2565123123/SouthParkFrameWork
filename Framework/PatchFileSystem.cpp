#include "PatchFileSystem.h"
#include "framework.h"
#include "Config.h"
#include "Notification.h"

#include <windows.h>
#include <vector>
#include <string>

struct PatchEntry
{
    std::wstring NameW;
    std::string NameA;
    std::wstring Path;
    bool Enabled;
};

static std::vector<PatchEntry> g_Patches;

static std::wstring PatchesFolder()
{
    return Framework_GetFolder() + L"\\Patches";
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

static std::string ConfigSectionForPatch(const std::string& name)
{
    return "Patch_" + name;
}

static bool FileExistsW(const std::wstring& path)
{
    DWORD attr = GetFileAttributesW(path.c_str());

    return
        attr != INVALID_FILE_ATTRIBUTES &&
        !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static void DetectPatches()
{
    g_Patches.clear();

    std::wstring search = PatchesFolder() + L"\\*";

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(search.c_str(), &data);

    if (find == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (wcscmp(data.cFileName, L".") == 0)
            continue;

        if (wcscmp(data.cFileName, L"..") == 0)
            continue;

        PatchEntry patch;
        patch.NameW = data.cFileName;
        patch.NameA = WideToAnsi(patch.NameW);
        patch.Path = PatchesFolder() + L"\\" + data.cFileName;

        std::string section = ConfigSectionForPatch(patch.NameA);

        // IMPORTANT:
        // New file patches are disabled by default.
        patch.Enabled = Config_GetBool(section.c_str(), "Enabled", false);

        g_Patches.push_back(patch);

    } while (FindNextFileW(find, &data));

    FindClose(find);
}

void PatchFileSystem_Init()
{
    CreateDirectoryW(PatchesFolder().c_str(), NULL);

    DetectPatches();

    Notification_Push("[PATCHFS] File patches initialized");
}

void PatchFileSystem_Reload()
{
    DetectPatches();

    Notification_Push("[PATCHFS] File patches refreshed");
}

int PatchFileSystem_GetPatchCount()
{
    return (int)g_Patches.size();
}

const char* PatchFileSystem_GetPatchName(int index)
{
    if (index < 0 || index >= (int)g_Patches.size())
        return "";

    return g_Patches[index].NameA.c_str();
}

bool PatchFileSystem_IsPatchEnabled(int index)
{
    if (index < 0 || index >= (int)g_Patches.size())
        return false;

    return g_Patches[index].Enabled;
}

void PatchFileSystem_TogglePatch(int index)
{
    if (index < 0 || index >= (int)g_Patches.size())
        return;

    g_Patches[index].Enabled = !g_Patches[index].Enabled;

    std::string section =
        ConfigSectionForPatch(g_Patches[index].NameA);

    Config_SetBool(
        section.c_str(),
        "Enabled",
        g_Patches[index].Enabled
    );

    Notification_Push(
        g_Patches[index].Enabled
        ? "[PATCHFS] File patch enabled"
        : "[PATCHFS] File patch disabled"
    );
}

bool PatchFileSystem_TryResolveW(
    const wchar_t* relativePath,
    wchar_t* outPath,
    int outCount
)
{
    if (!relativePath || !outPath || outCount <= 0)
        return false;

    for (size_t i = 0; i < g_Patches.size(); i++)
    {
        if (!g_Patches[i].Enabled)
            continue;

        std::wstring test =
            g_Patches[i].Path +
            L"\\" +
            relativePath;

        if (FileExistsW(test))
        {
            wcscpy_s(outPath, outCount, test.c_str());
            return true;
        }
    }

    return false;
}