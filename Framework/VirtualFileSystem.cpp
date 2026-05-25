#include "VirtualFileSystem.h"
#include "framework.h"
#include "MinHook.h"
#include "PatchFileSystem.h"

#include <windows.h>
#include <string>

typedef HANDLE(WINAPI* CreateFileA_t)(
    LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

typedef HANDLE(WINAPI* CreateFileW_t)(
    LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

static CreateFileA_t Original_CreateFileA = NULL;
static CreateFileW_t Original_CreateFileW = NULL;

static bool g_Enabled = true;

static bool FileExistsW(const std::wstring& path)
{
    DWORD attr = GetFileAttributesW(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

static std::wstring NormalizeSlash(std::wstring path)
{
    for (size_t i = 0; i < path.size(); i++)
    {
        if (path[i] == L'/')
            path[i] = L'\\';
    }

    return path;
}

static std::wstring GetRelativeGamePathW(const wchar_t* fileName)
{
    if (!fileName)
        return L"";

    std::wstring input = NormalizeSlash(fileName);
    std::wstring exe = NormalizeSlash(Framework_GetExeFolder());

    if (input.find(exe) == 0)
    {
        std::wstring rel = input.substr(exe.size());

        while (!rel.empty() && (rel[0] == L'\\' || rel[0] == L'/'))
            rel.erase(rel.begin());

        return rel;
    }

    return input;
}

static bool TryResolveModFileW(const wchar_t* requested, std::wstring& outPath)
{
    if (!g_Enabled || !requested)
        return false;

    std::wstring rel = GetRelativeGamePathW(requested);

    if (rel.empty())
        return false;

    std::wstring modsRoot = Framework_GetFolder() + L"\\Mods\\*";

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(modsRoot.c_str(), &data);

    if (find == INVALID_HANDLE_VALUE)
        return false;

    wchar_t patchPath[MAX_PATH * 2];

    if (PatchFileSystem_TryResolveW(
        rel.c_str(),
        patchPath,
        MAX_PATH * 2
    ))
    {
        outPath = patchPath;
        return true;
    }

    do
    {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (wcscmp(data.cFileName, L".") == 0)
            continue;

        if (wcscmp(data.cFileName, L"..") == 0)
            continue;

        std::wstring test =
            Framework_GetFolder() +
            L"\\Mods\\" +
            data.cFileName +
            L"\\" +
            rel;

        if (FileExistsW(test))
        {
            outPath = test;
            FindClose(find);
            return true;
        }

    } while (FindNextFileW(find, &data));

    FindClose(find);
    return false;
}

static HANDLE WINAPI Hooked_CreateFileW(
    LPCWSTR fileName,
    DWORD desiredAccess,
    DWORD shareMode,
    LPSECURITY_ATTRIBUTES security,
    DWORD creationDisposition,
    DWORD flags,
    HANDLE templateFile
)
{
    std::wstring redirected;

    if (TryResolveModFileW(fileName, redirected))
    {
        Framework_Log("[VFS] Redirected file");

        return Original_CreateFileW(
            redirected.c_str(),
            desiredAccess,
            shareMode,
            security,
            creationDisposition,
            flags,
            templateFile
        );
    }

    return Original_CreateFileW(
        fileName,
        desiredAccess,
        shareMode,
        security,
        creationDisposition,
        flags,
        templateFile
    );
}

static HANDLE WINAPI Hooked_CreateFileA(
    LPCSTR fileName,
    DWORD desiredAccess,
    DWORD shareMode,
    LPSECURITY_ATTRIBUTES security,
    DWORD creationDisposition,
    DWORD flags,
    HANDLE templateFile
)
{
    if (!fileName)
    {
        return Original_CreateFileA(
            fileName,
            desiredAccess,
            shareMode,
            security,
            creationDisposition,
            flags,
            templateFile
        );
    }

    wchar_t wide[MAX_PATH * 2];
    MultiByteToWideChar(CP_ACP, 0, fileName, -1, wide, MAX_PATH * 2);

    std::wstring redirected;

    if (TryResolveModFileW(wide, redirected))
    {
        char ansi[MAX_PATH * 2];
        WideCharToMultiByte(CP_ACP, 0, redirected.c_str(), -1, ansi, MAX_PATH * 2, NULL, NULL);

        Framework_Log("[VFS] Redirected file");

        return Original_CreateFileA(
            ansi,
            desiredAccess,
            shareMode,
            security,
            creationDisposition,
            flags,
            templateFile
        );
    }

    return Original_CreateFileA(
        fileName,
        desiredAccess,
        shareMode,
        security,
        creationDisposition,
        flags,
        templateFile
    );
}

void VFS_Init()
{
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");

    if (!kernel32)
    {
        Framework_Log("[VFS] kernel32.dll not found");
        return;
    }

    void* createFileA = GetProcAddress(kernel32, "CreateFileA");
    void* createFileW = GetProcAddress(kernel32, "CreateFileW");

    if (createFileA)
    {
        if (MH_CreateHook(
            createFileA,
            &Hooked_CreateFileA,
            reinterpret_cast<void**>(&Original_CreateFileA)
        ) == MH_OK)
        {
            MH_EnableHook(createFileA);
            Framework_Log("[VFS] CreateFileA hooked");
        }
    }

    if (createFileW)
    {
        if (MH_CreateHook(
            createFileW,
            &Hooked_CreateFileW,
            reinterpret_cast<void**>(&Original_CreateFileW)
        ) == MH_OK)
        {
            MH_EnableHook(createFileW);
            Framework_Log("[VFS] CreateFileW hooked");
        }
    }

    CreateDirectoryW((Framework_GetFolder() + L"\\Mods").c_str(), NULL);

    Framework_Log("[VFS] Virtual file system initialized");
}

void VFS_Shutdown()
{}

bool VFS_IsEnabled()
{
    return g_Enabled;
}

void VFS_SetEnabled(bool enabled)
{
    g_Enabled = enabled;

    Framework_Log(
        g_Enabled
        ? "[VFS] Virtual files enabled"
        : "[VFS] Virtual files disabled"
    );
}