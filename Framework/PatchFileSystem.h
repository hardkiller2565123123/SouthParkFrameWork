#pragma once

void PatchFileSystem_Init();
void PatchFileSystem_Reload();

int PatchFileSystem_GetPatchCount();
const char* PatchFileSystem_GetPatchName(int index);

bool PatchFileSystem_IsPatchEnabled(int index);
void PatchFileSystem_TogglePatch(int index);

bool PatchFileSystem_TryResolveW(const wchar_t* relativePath, wchar_t* outPath, int outCount);