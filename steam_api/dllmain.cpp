#include "STDInc.h"
#include <stdarg.h>

HMODULE g_RealSteam = NULL;

extern "C" __declspec(dllexport) void* g_pSteamClientGameServer = NULL;

static HMODULE g_LoggerDll = NULL;
static HMODULE g_FrameworkDll = NULL;

typedef void(__cdecl* Logger_Init_t)();
typedef void(__cdecl* Logger_Shutdown_t)();
typedef void(__cdecl* Logger_Log_t)(const char*);
typedef void(__cdecl* Logger_LogCall_t)(const char*, void*);
typedef void(__cdecl* Logger_LogMissing_t)(const char*);
typedef void(__cdecl* Logger_SetStatus_t)(const char*);

static Logger_Init_t g_LoggerInit = NULL;
static Logger_Shutdown_t g_LoggerShutdown = NULL;
static Logger_Log_t g_LoggerLog = NULL;
static Logger_LogCall_t g_LoggerLogCall = NULL;
static Logger_LogMissing_t g_LoggerLogMissing = NULL;
static Logger_SetStatus_t g_LoggerSetStatus = NULL;

std::wstring GetExeFolder()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    std::wstring p = path;
    size_t slash = p.find_last_of(L"\\/");

    if (slash != std::wstring::npos)
        p = p.substr(0, slash);

    return p;
}

std::wstring GetFrameworkFolder()
{
    return GetExeFolder() + L"\\SouthParkFrameWork";
}

void ProxyLogger_Init()
{
    if (g_LoggerDll)
        return;

    std::wstring loggerPath = GetFrameworkFolder() + L"\\Logger.dll";

    g_LoggerDll = LoadLibraryW(loggerPath.c_str());

    if (!g_LoggerDll)
        return;

    g_LoggerInit =
        (Logger_Init_t)GetProcAddress(g_LoggerDll, "Logger_Init");

    g_LoggerShutdown =
        (Logger_Shutdown_t)GetProcAddress(g_LoggerDll, "Logger_Shutdown");

    g_LoggerLog =
        (Logger_Log_t)GetProcAddress(g_LoggerDll, "Logger_Log");

    g_LoggerLogCall =
        (Logger_LogCall_t)GetProcAddress(g_LoggerDll, "Logger_LogCall");

    g_LoggerLogMissing =
        (Logger_LogMissing_t)GetProcAddress(g_LoggerDll, "Logger_LogMissing");

    g_LoggerSetStatus =
        (Logger_SetStatus_t)GetProcAddress(g_LoggerDll, "Logger_SetStatus");

    if (g_LoggerInit)
        g_LoggerInit();
}

void ProxyLogger_Shutdown()
{
    if (g_LoggerShutdown)
        g_LoggerShutdown();

    g_LoggerInit = NULL;
    g_LoggerShutdown = NULL;
    g_LoggerLog = NULL;
    g_LoggerLogCall = NULL;
    g_LoggerLogMissing = NULL;
    g_LoggerSetStatus = NULL;

    g_LoggerDll = NULL;
}

void ProxyLogger_Log(const char* text)
{
    if (g_LoggerLog)
        g_LoggerLog(text);
}

void ProxyLogger_LogCall(const char* functionName, void* address)
{
    if (g_LoggerLogCall)
        g_LoggerLogCall(functionName, address);
}

void ProxyLogger_LogMissing(const char* functionName)
{
    if (g_LoggerLogMissing)
        g_LoggerLogMissing(functionName);
}

void ProxyLogger_SetStatus(const char* text)
{
    if (g_LoggerSetStatus)
        g_LoggerSetStatus(text);
}

static LONG CALLBACK CrashHandler(EXCEPTION_POINTERS* e)
{
    DWORD code = e->ExceptionRecord->ExceptionCode;

    if (code == 0x406D1388)
        return EXCEPTION_CONTINUE_SEARCH;

    char buffer[512];

    sprintf_s(
        buffer,
        sizeof(buffer),
        "[REAL CRASH] Code=0x%08X Address=0x%p EIP=0x%p",
        code,
        e->ExceptionRecord->ExceptionAddress,
        (void*)e->ContextRecord->Eip
    );

    ProxyLogger_Log(buffer);

    return EXCEPTION_CONTINUE_SEARCH;
}

HMODULE LoadRealSteam()
{
    if (g_RealSteam)
        return g_RealSteam;

    std::wstring folder = GetFrameworkFolder();
    std::wstring realDll = folder + L"\\SteamOnline.dll";

    SetDllDirectoryW(folder.c_str());

    ProxyLogger_Log("[BOOT] Loading SouthParkFrameWork\\SteamOnline.dll");

    g_RealSteam = LoadLibraryExW(
        realDll.c_str(),
        NULL,
        LOAD_WITH_ALTERED_SEARCH_PATH
    );

    if (!g_RealSteam)
    {
        char buffer[256];

        sprintf_s(
            buffer,
            sizeof(buffer),
            "[ERROR] Failed to load SteamOnline.dll. GetLastError=%lu",
            GetLastError()
        );

        ProxyLogger_Log(buffer);
    }
    else
    {
        ProxyLogger_Log("[OK] Loaded SteamOnline.dll");

        g_pSteamClientGameServer =
            (void*)GetProcAddress(g_RealSteam, "g_pSteamClientGameServer");

        if (g_pSteamClientGameServer)
            ProxyLogger_Log("[OK] g_pSteamClientGameServer linked");
        else
            ProxyLogger_Log("[WARN] g_pSteamClientGameServer not found");
    }

    return g_RealSteam;
}

static void LoadFrameworkDll()
{
    if (g_FrameworkDll)
        return;

    std::wstring path = GetFrameworkFolder() + L"\\Framework.dll";

    g_FrameworkDll = LoadLibraryW(path.c_str());

    if (g_FrameworkDll)
    {
        ProxyLogger_Log("[FRAMEWORK] Framework.dll loaded");
    }
    else
    {
        DWORD err = GetLastError();

        char buffer[256];

        sprintf_s(
            buffer,
            sizeof(buffer),
            "[FRAMEWORK] Framework.dll failed. GetLastError=%lu",
            err
        );

        ProxyLogger_Log(buffer);
    }
}

FARPROC GetSteamExport(const char* name)
{
    HMODULE steam = LoadRealSteam();

    if (!steam)
    {
        ProxyLogger_Log("[ERROR] SteamOnline.dll is not loaded");
        return NULL;
    }

    FARPROC proc = GetProcAddress(steam, name);

    if (!proc)
    {
        ProxyLogger_LogMissing(name);
        return NULL;
    }

    ProxyLogger_LogCall(name, proc);

    return proc;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        ProxyLogger_Init();
        ProxyLogger_Log("SouthParkFrameWork steam_api proxy loaded");

        AddVectoredExceptionHandler(1, CrashHandler);

        LoadRealSteam();
        LoadFrameworkDll();

        ProxyLogger_SetStatus("Running");
    }

    if (reason == DLL_PROCESS_DETACH)
    {
        ProxyLogger_SetStatus("Game closed");
        ProxyLogger_Shutdown();
    }

    return TRUE;
}