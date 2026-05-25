#pragma once

#include <windows.h>
#include <stdint.h>

void PatchManager_Init();

uintptr_t GameBase();
uintptr_t RVA(uintptr_t rva);

bool WriteMemory(void* address, const void* data, size_t size);
bool WriteBytes(uintptr_t address, const unsigned char* bytes, size_t size);
bool WriteNop(uintptr_t address, size_t size);