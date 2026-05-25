#pragma once

extern "C"
{
    __declspec(dllexport) void __cdecl Logger_Init();
    __declspec(dllexport) void __cdecl Logger_Shutdown();

    __declspec(dllexport) void __cdecl Logger_Log(const char* text);
    __declspec(dllexport) void __cdecl Logger_LogCall(const char* functionName, void* address);
    __declspec(dllexport) void __cdecl Logger_LogMissing(const char* functionName);
    __declspec(dllexport) void __cdecl Logger_SetStatus(const char* text);
}