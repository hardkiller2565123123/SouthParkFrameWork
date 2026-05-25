#pragma once
#include <d3d9.h>

void Overlay_Init();
void Overlay_Shutdown();

void Overlay_OnLostDevice();
void Overlay_OnResetDevice();
void Overlay_Toggle();

void Overlay_Render(LPDIRECT3DDEVICE9 device);