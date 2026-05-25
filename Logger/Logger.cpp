#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>

struct RuntimeLine
{
    std::string Text;
    bool Success;
};

static CRITICAL_SECTION g_Lock;
static bool g_Ready = false;
static bool g_HasConsole = false;

static std::vector<RuntimeLine> g_Lines;

static std::wstring GetExeFolder()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    std::wstring p = path;
    size_t slash = p.find_last_of(L"\\/");

    if (slash != std::wstring::npos)
        p = p.substr(0, slash);

    return p;
}

static std::wstring GetFrameworkFolder()
{
    return GetExeFolder() + L"\\SouthParkFrameWork";
}

static void WriteLogLine(const char* text)
{
    CreateDirectoryW(GetFrameworkFolder().c_str(), NULL);

    std::wstring logPath =
        GetFrameworkFolder() + L"\\SouthParkFrameWork.log";

    std::ofstream file(logPath.c_str(), std::ios::app);

    if (file.is_open())
        file << text << std::endl;
}

static bool HasLine(const char* text)
{
    for (size_t i = 0; i < g_Lines.size(); i++)
    {
        if (_stricmp(g_Lines[i].Text.c_str(), text) == 0)
            return true;
    }

    return false;
}

static void SetNormalColor()
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(
        out,
        FOREGROUND_RED |
        FOREGROUND_GREEN |
        FOREGROUND_BLUE |
        FOREGROUND_INTENSITY
    );
}

static void SetTitleColor()
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(
        out,
        FOREGROUND_GREEN |
        FOREGROUND_INTENSITY
    );
}

static void SetStatusColor(bool success)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    if (success)
    {
        SetConsoleTextAttribute(
            out,
            FOREGROUND_GREEN |
            FOREGROUND_INTENSITY
        );
    }
    else
    {
        SetConsoleTextAttribute(
            out,
            FOREGROUND_RED |
            FOREGROUND_INTENSITY
        );
    }
}

static void PrintRuntimeLine(const char* text, bool success)
{
    if (!g_HasConsole || !text)
        return;

    SetStatusColor(success);
    printf("%s\n", text);
    SetNormalColor();
}

static void AddRuntimeLine(const char* type, const char* name, bool success)
{
    if (!type || !name)
        return;

    char buffer[256];

    sprintf_s(
        buffer,
        sizeof(buffer),
        "[%-6s] %-32s %s",
        type,
        name,
        success ? "initialized" : "failed"
    );

    if (HasLine(buffer))
        return;

    RuntimeLine line;
    line.Text = buffer;
    line.Success = success;

    g_Lines.push_back(line);

    WriteLogLine(buffer);
    PrintRuntimeLine(buffer, success);
}

static BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType)
{
    if (
        ctrlType == CTRL_CLOSE_EVENT ||
        ctrlType == CTRL_C_EVENT ||
        ctrlType == CTRL_BREAK_EVENT ||
        ctrlType == CTRL_LOGOFF_EVENT ||
        ctrlType == CTRL_SHUTDOWN_EVENT
        )
    {
        ExitProcess(0);
        return TRUE;
    }

    return FALSE;
}

static DWORD WINAPI ConsoleWatchThread(LPVOID)
{
    while (true)
    {
        HWND hwnd = GetConsoleWindow();

        if (!hwnd || !IsWindow(hwnd))
        {
            ExitProcess(0);
            return 0;
        }

        Sleep(250);
    }

    return 0;
}

static void SetupConsole()
{
    AllocConsole();

    FILE* f;

    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);

    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    SetConsoleTitleA("SouthParkFrameWork");

    HWND hwnd = GetConsoleWindow();

    if (hwnd)
    {
        LONG style = GetWindowLong(hwnd, GWL_STYLE);

        style &= ~WS_MAXIMIZEBOX;
        style &= ~WS_SIZEBOX;

        SetWindowLong(hwnd, GWL_STYLE, style);

        MoveWindow(hwnd, 80, 80, 700, 800, TRUE);
    }

    SetNormalColor();

    HANDLE thread = CreateThread(
        NULL,
        0,
        ConsoleWatchThread,
        NULL,
        0,
        NULL
    );

    if (thread)
        CloseHandle(thread);

    g_HasConsole = true;
}

extern "C" __declspec(dllexport)
void __cdecl Logger_Init()
{
    if (g_Ready)
        return;

    InitializeCriticalSection(&g_Lock);
    g_Ready = true;

    SetupConsole();

    SetTitleColor();
    printf("SouthParkFrameWork\n");

    SetNormalColor();
    printf("------------------\n");

    WriteLogLine("========================================");
    WriteLogLine("SouthParkFrameWork initialized");

    AddRuntimeLine("DLL", "Logger.dll", true);
    AddRuntimeLine("DLL", "steam_api.dll", true);
}

extern "C" __declspec(dllexport)
void __cdecl Logger_Shutdown()
{
    if (!g_Ready)
        return;

    EnterCriticalSection(&g_Lock);

    WriteLogLine("SouthParkFrameWork shutdown");

    LeaveCriticalSection(&g_Lock);

    DeleteCriticalSection(&g_Lock);
    g_Ready = false;
}

extern "C" __declspec(dllexport)
void __cdecl Logger_Log(const char* text)
{
    if (!g_Ready || !text)
        return;

    EnterCriticalSection(&g_Lock);

    WriteLogLine(text);

    bool isFailure =
        strstr(text, "[ERROR]") ||
        strstr(text, "[MISSING]") ||
        strstr(text, "[REAL CRASH]") ||
        strstr(text, "failed") ||
        strstr(text, "FAILED") ||
        strstr(text, "Failed");

    if (strstr(text, "Loaded SteamOnline.dll"))
    {
        AddRuntimeLine("DLL", "SteamOnline.dll", true);
    }
    else if (strstr(text, "Failed to load SteamOnline.dll"))
    {
        AddRuntimeLine("DLL", "SteamOnline.dll", false);
    }
    else if (strstr(text, "Framework.dll loaded"))
    {
        AddRuntimeLine("DLL", "Framework.dll", true);
    }
    else if (strstr(text, "Framework.dll failed"))
    {
        AddRuntimeLine("DLL", "Framework.dll", false);
    }
    else if (strstr(text, "g_pSteamClientGameServer linked"))
    {
        AddRuntimeLine("STEAM", "SteamClientGameServer", true);
    }
    else if (strstr(text, "g_pSteamClientGameServer not found"))
    {
        AddRuntimeLine("STEAM", "SteamClientGameServer", false);
    }
    else if (
        strstr(text, "[FRAMEWORK]") ||
        strstr(text, "[HOOK]") ||
        strstr(text, "[OVERLAY]") ||
        strstr(text, "[WINDOW]") ||
        strstr(text, "[PATCH]") ||
        strstr(text, "[INPUT]") ||
        strstr(text, "[SCAN]") ||
        strstr(text, "[ERROR]") ||
        strstr(text, "[MISSING]") ||
        strstr(text, "[REAL CRASH]")
        )
    {
        PrintRuntimeLine(text, !isFailure);
    }

    LeaveCriticalSection(&g_Lock);
}

extern "C" __declspec(dllexport)
void __cdecl Logger_LogCall(const char* functionName, void* address)
{
    if (!g_Ready || !functionName)
        return;

    EnterCriticalSection(&g_Lock);

    if (strcmp(functionName, "SteamAPI_Init") == 0)
        AddRuntimeLine("STEAM", "SteamAPI_Init", true);

    else if (strcmp(functionName, "SteamClient") == 0)
        AddRuntimeLine("STEAM", "SteamClient", true);

    else if (strcmp(functionName, "SteamUser") == 0)
        AddRuntimeLine("STEAM", "SteamUser", true);

    else if (strcmp(functionName, "SteamFriends") == 0)
        AddRuntimeLine("STEAM", "SteamFriends", true);

    else if (strcmp(functionName, "SteamUtils") == 0)
        AddRuntimeLine("STEAM", "SteamUtils", true);

    else if (strcmp(functionName, "SteamApps") == 0)
        AddRuntimeLine("STEAM", "SteamApps", true);

    else if (strcmp(functionName, "SteamNetworking") == 0)
        AddRuntimeLine("STEAM", "SteamNetworking", true);

    else if (strcmp(functionName, "SteamRemoteStorage") == 0)
        AddRuntimeLine("STEAM", "SteamRemoteStorage", true);

    else if (strcmp(functionName, "SteamGameServer") == 0)
        AddRuntimeLine("STEAM", "SteamGameServer", true);

    LeaveCriticalSection(&g_Lock);
}

extern "C" __declspec(dllexport)
void __cdecl Logger_LogMissing(const char* functionName)
{
    if (!g_Ready || !functionName)
        return;

    EnterCriticalSection(&g_Lock);

    AddRuntimeLine("MISSING", functionName, false);

    LeaveCriticalSection(&g_Lock);
}

extern "C" __declspec(dllexport)
void __cdecl Logger_SetStatus(const char* text)
{
    if (!g_Ready || !text)
        return;

    WriteLogLine(text);
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_DETACH)
    {
        if (g_Ready)
            Logger_Shutdown();
    }

    return TRUE;
}