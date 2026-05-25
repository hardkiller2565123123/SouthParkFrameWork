#include "HookManager.h"
#include "framework.h"
#include "Overlay.h"
#include "MinHook.h"

#include <windows.h>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")

typedef IDirect3D9* (WINAPI* Direct3DCreate9_t)(UINT);
static Direct3DCreate9_t Original_Direct3DCreate9 = NULL;

typedef HRESULT(APIENTRY* CreateDevice_t)(
    IDirect3D9*,
    UINT,
    D3DDEVTYPE,
    HWND,
    DWORD,
    D3DPRESENT_PARAMETERS*,
    IDirect3DDevice9**
    );

static CreateDevice_t Original_CreateDevice = NULL;

typedef HRESULT(APIENTRY* EndScene_t)(LPDIRECT3DDEVICE9);
typedef HRESULT(APIENTRY* Reset_t)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);

static EndScene_t Original_EndScene = NULL;
static Reset_t Original_Reset = NULL;

static bool g_DeviceHooked = false;

static HRESULT APIENTRY Hooked_EndScene(LPDIRECT3DDEVICE9 device)
{
    Overlay_Render(device);
    return Original_EndScene(device);
}

static HRESULT APIENTRY Hooked_Reset(
    LPDIRECT3DDEVICE9 device,
    D3DPRESENT_PARAMETERS* params
)
{
    Overlay_OnLostDevice();

    HRESULT result = Original_Reset(device, params);

    Overlay_OnResetDevice();

    return result;
}

static void HookRealDevice(IDirect3DDevice9* device)
{
    if (!device || g_DeviceHooked)
        return;

    void** table = *reinterpret_cast<void***>(device);

    void* reset = table[16];
    void* endScene = table[42];

    if (MH_CreateHook(
        endScene,
        &Hooked_EndScene,
        reinterpret_cast<void**>(&Original_EndScene)
    ) == MH_OK)
    {
        MH_EnableHook(endScene);
        Framework_Log("[HOOK] REAL EndScene hooked");
    }
    else
    {
        Framework_Log("[HOOK] REAL EndScene failed");
    }

    if (MH_CreateHook(
        reset,
        &Hooked_Reset,
        reinterpret_cast<void**>(&Original_Reset)
    ) == MH_OK)
    {
        MH_EnableHook(reset);
        Framework_Log("[HOOK] REAL Reset hooked");
    }
    else
    {
        Framework_Log("[HOOK] REAL Reset failed");
    }

    g_DeviceHooked = true;
    Framework_Log("[HOOK] REAL D3D9 device initialized");
}

static HRESULT APIENTRY Hooked_CreateDevice(
    IDirect3D9* self,
    UINT adapter,
    D3DDEVTYPE deviceType,
    HWND hwnd,
    DWORD flags,
    D3DPRESENT_PARAMETERS* params,
    IDirect3DDevice9** outDevice
)
{
    HRESULT result = Original_CreateDevice(
        self,
        adapter,
        deviceType,
        hwnd,
        flags,
        params,
        outDevice
    );

    if (SUCCEEDED(result) && outDevice && *outDevice)
    {
        Framework_Log("[HOOK] CreateDevice succeeded");
        HookRealDevice(*outDevice);
    }

    return result;
}

static IDirect3D9* WINAPI Hooked_Direct3DCreate9(UINT sdk)
{
    IDirect3D9* d3d = Original_Direct3DCreate9(sdk);

    if (!d3d)
        return d3d;

    Framework_Log("[HOOK] Direct3DCreate9 intercepted");

    void** table = *reinterpret_cast<void***>(d3d);
    void* createDevice = table[16];

    if (MH_CreateHook(
        createDevice,
        &Hooked_CreateDevice,
        reinterpret_cast<void**>(&Original_CreateDevice)
    ) == MH_OK)
    {
        MH_EnableHook(createDevice);
        Framework_Log("[HOOK] CreateDevice hooked");
    }

    return d3d;
}

void HookManager_Init()
{
    Framework_Log("[HOOK] HookManager initialized");

    HMODULE d3d9 = GetModuleHandleA("d3d9.dll");

    if (!d3d9)
        d3d9 = LoadLibraryA("d3d9.dll");

    if (!d3d9)
    {
        Framework_Log("[HOOK] d3d9.dll not found");
        return;
    }

    void* target = GetProcAddress(d3d9, "Direct3DCreate9");

    if (!target)
    {
        Framework_Log("[HOOK] Direct3DCreate9 missing");
        return;
    }

    if (MH_CreateHook(
        target,
        &Hooked_Direct3DCreate9,
        reinterpret_cast<void**>(&Original_Direct3DCreate9)
    ) == MH_OK)
    {
        MH_EnableHook(target);
        Framework_Log("[HOOK] Direct3DCreate9 hooked");
    }
}