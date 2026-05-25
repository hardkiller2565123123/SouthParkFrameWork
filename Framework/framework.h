#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

void Framework_Log(const char* text);

std::wstring Framework_GetExeFolder();
std::wstring Framework_GetFolder();

DWORD WINAPI Framework_MainThread(LPVOID);