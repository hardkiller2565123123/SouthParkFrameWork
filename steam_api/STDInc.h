#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

extern HMODULE g_RealSteam;

extern "C" __declspec(dllexport) void* g_pSteamClientGameServer;

std::wstring GetExeFolder();
std::wstring GetFrameworkFolder();

void ProxyLogger_Init();
void ProxyLogger_Shutdown();
void ProxyLogger_Log(const char* text);
void ProxyLogger_LogCall(const char* functionName, void* address);
void ProxyLogger_LogMissing(const char* functionName);
void ProxyLogger_SetStatus(const char* text);

HMODULE LoadRealSteam();
FARPROC GetSteamExport(const char* name);