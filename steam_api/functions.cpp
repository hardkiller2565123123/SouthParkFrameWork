#include "STDInc.h"

#define FORWARD_PTR(name) \
extern "C" __declspec(dllexport) void* __cdecl name() \
{ \
    FARPROC p = GetSteamExport(#name); \
    if (!p) return NULL; \
    typedef void* (__cdecl* Fn)(); \
    return ((Fn)p)(); \
}

#define FORWARD_BOOL(name) \
extern "C" __declspec(dllexport) bool __cdecl name() \
{ \
    FARPROC p = GetSteamExport(#name); \
    if (!p) return false; \
    typedef bool(__cdecl* Fn)(); \
    return ((Fn)p)(); \
}

#define FORWARD_INT(name) \
extern "C" __declspec(dllexport) int __cdecl name() \
{ \
    FARPROC p = GetSteamExport(#name); \
    if (!p) return 0; \
    typedef int(__cdecl* Fn)(); \
    return ((Fn)p)(); \
}

#define FORWARD_UINT(name) \
extern "C" __declspec(dllexport) unsigned int __cdecl name() \
{ \
    FARPROC p = GetSteamExport(#name); \
    if (!p) return 0; \
    typedef unsigned int(__cdecl* Fn)(); \
    return ((Fn)p)(); \
}

#define FORWARD_VOID(name) \
extern "C" __declspec(dllexport) void __cdecl name() \
{ \
    FARPROC p = GetSteamExport(#name); \
    if (!p) return; \
    typedef void(__cdecl* Fn)(); \
    ((Fn)p)(); \
}

FORWARD_INT(GetHSteamPipe)
FORWARD_INT(GetHSteamUser)

FORWARD_INT(SteamAPI_GetHSteamPipe)
FORWARD_INT(SteamAPI_GetHSteamUser)
FORWARD_PTR(SteamAPI_GetSteamInstallPath)
FORWARD_BOOL(SteamAPI_Init)
FORWARD_BOOL(SteamAPI_InitSafe)
FORWARD_BOOL(SteamAPI_IsSteamRunning)
FORWARD_VOID(SteamAPI_RunCallbacks)
FORWARD_VOID(SteamAPI_Shutdown)

FORWARD_PTR(SteamApps)
FORWARD_PTR(SteamClient)
FORWARD_PTR(SteamContentServer)
FORWARD_PTR(SteamContentServerUtils)
FORWARD_VOID(SteamContentServer_RunCallbacks)
FORWARD_VOID(SteamContentServer_Shutdown)
FORWARD_PTR(SteamFriends)
FORWARD_PTR(SteamGameServer)
FORWARD_PTR(SteamGameServerApps)
FORWARD_PTR(SteamGameServerHTTP)
FORWARD_PTR(SteamGameServerNetworking)
FORWARD_PTR(SteamGameServerStats)
FORWARD_PTR(SteamGameServerUtils)
FORWARD_BOOL(SteamGameServer_BSecure)
FORWARD_INT(SteamGameServer_GetHSteamPipe)
FORWARD_INT(SteamGameServer_GetHSteamUser)
FORWARD_UINT(SteamGameServer_GetIPCCallCount)
FORWARD_PTR(SteamGameServer_GetSteamID)
FORWARD_BOOL(SteamGameServer_InitSafe)
FORWARD_VOID(SteamGameServer_RunCallbacks)
FORWARD_VOID(SteamGameServer_Shutdown)

FORWARD_PTR(SteamHTTP)
FORWARD_PTR(SteamMatchmaking)
FORWARD_PTR(SteamMatchmakingServers)
FORWARD_PTR(SteamNetworking)
FORWARD_PTR(SteamRemoteStorage)
FORWARD_PTR(SteamScreenshots)
FORWARD_PTR(SteamUser)
FORWARD_PTR(SteamUserStats)
FORWARD_PTR(SteamUtils)

FORWARD_INT(Steam_GetHSteamUserCurrent)
FORWARD_VOID(Steam_RegisterInterfaceFuncs)
FORWARD_VOID(Steam_RunCallbacks)

extern "C" __declspec(dllexport)
bool __cdecl SteamAPI_RestartAppIfNecessary(unsigned int appId)
{
    FARPROC p = GetSteamExport("SteamAPI_RestartAppIfNecessary");
    if (!p) return false;

    typedef bool(__cdecl* Fn)(unsigned int);
    return ((Fn)p)(appId);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_RegisterCallResult(void* callback, unsigned long long call)
{
    FARPROC p = GetSteamExport("SteamAPI_RegisterCallResult");
    if (!p) return;

    typedef void(__cdecl* Fn)(void*, unsigned long long);
    ((Fn)p)(callback, call);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_RegisterCallback(void* callback, int callbackId)
{
    FARPROC p = GetSteamExport("SteamAPI_RegisterCallback");
    if (!p) return;

    typedef void(__cdecl* Fn)(void*, int);
    ((Fn)p)(callback, callbackId);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_UnregisterCallResult(void* callback, unsigned long long call)
{
    FARPROC p = GetSteamExport("SteamAPI_UnregisterCallResult");
    if (!p) return;

    typedef void(__cdecl* Fn)(void*, unsigned long long);
    ((Fn)p)(callback, call);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_UnregisterCallback(void* callback)
{
    FARPROC p = GetSteamExport("SteamAPI_UnregisterCallback");
    if (!p) return;

    typedef void(__cdecl* Fn)(void*);
    ((Fn)p)(callback);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_SetBreakpadAppID(unsigned int appId)
{
    FARPROC p = GetSteamExport("SteamAPI_SetBreakpadAppID");
    if (!p) return;

    typedef void(__cdecl* Fn)(unsigned int);
    ((Fn)p)(appId);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_SetMiniDumpComment(const char* comment)
{
    FARPROC p = GetSteamExport("SteamAPI_SetMiniDumpComment");
    if (!p) return;

    typedef void(__cdecl* Fn)(const char*);
    ((Fn)p)(comment);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_SetTryCatchCallbacks(bool enabled)
{
    FARPROC p = GetSteamExport("SteamAPI_SetTryCatchCallbacks");
    if (!p) return;

    typedef void(__cdecl* Fn)(bool);
    ((Fn)p)(enabled);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_UseBreakpadCrashHandler(
    const char* version,
    const char* date,
    const char* time,
    bool fullMemoryDumps,
    void* context,
    void* preMinidumpCallback
)
{
    FARPROC p = GetSteamExport("SteamAPI_UseBreakpadCrashHandler");
    if (!p) return;

    typedef void(__cdecl* Fn)(const char*, const char*, const char*, bool, void*, void*);
    ((Fn)p)(version, date, time, fullMemoryDumps, context, preMinidumpCallback);
}

extern "C" __declspec(dllexport)
void __cdecl SteamAPI_WriteMiniDump(unsigned int code, void* info, unsigned int build)
{
    FARPROC p = GetSteamExport("SteamAPI_WriteMiniDump");
    if (!p) return;

    typedef void(__cdecl* Fn)(unsigned int, void*, unsigned int);
    ((Fn)p)(code, info, build);
}

extern "C" __declspec(dllexport)
bool __cdecl SteamContentServer_Init(unsigned int ip, unsigned short port)
{
    FARPROC p = GetSteamExport("SteamContentServer_Init");
    if (!p) return false;

    typedef bool(__cdecl* Fn)(unsigned int, unsigned short);
    return ((Fn)p)(ip, port);
}

extern "C" __declspec(dllexport)
bool __cdecl SteamGameServer_Init(
    unsigned int ip,
    unsigned short steamPort,
    unsigned short gamePort,
    unsigned short queryPort,
    unsigned int serverMode,
    const char* version
)
{
    FARPROC p = GetSteamExport("SteamGameServer_Init");
    if (!p) return false;

    typedef bool(__cdecl* Fn)(
        unsigned int,
        unsigned short,
        unsigned short,
        unsigned short,
        unsigned int,
        const char*
        );

    return ((Fn)p)(
        ip,
        steamPort,
        gamePort,
        queryPort,
        serverMode,
        version
        );
}