#include "PatchManager.h"
#include "framework.h"

uintptr_t GameBase()
{
    return (uintptr_t)GetModuleHandleW(NULL);
}

uintptr_t RVA(uintptr_t rva)
{
    return GameBase() + rva;
}

bool WriteMemory(void* address, const void* data, size_t size)
{
    if (!address || !data || size == 0)
        return false;

    DWORD oldProtect = 0;

    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    memcpy(address, data, size);

    DWORD temp = 0;
    VirtualProtect(address, size, oldProtect, &temp);

    FlushInstructionCache(GetCurrentProcess(), address, size);

    return true;
}

bool WriteBytes(uintptr_t address, const unsigned char* bytes, size_t size)
{
    return WriteMemory((void*)address, bytes, size);
}

bool WriteNop(uintptr_t address, size_t size)
{
    if (size == 0)
        return false;

    unsigned char* nops = new unsigned char[size];

    for (size_t i = 0; i < size; i++)
        nops[i] = 0x90;

    bool result = WriteBytes(address, nops, size);

    delete[] nops;

    return result;
}

void PatchManager_Init()
{
    Framework_Log("[PATCH] PatchManager initialized");
}