#pragma once
#include <d3d9.h>

void Overlay_Init();
void Overlay_Shutdown();

void Overlay_OnLostDevice();
void Overlay_OnResetDevice();

void Overlay_Render(LPDIRECT3DDEVICE9 device);

void Overlay_ToggleHud();
void Overlay_ToggleMenu();

void Overlay_MenuUp();
void Overlay_MenuDown();
void Overlay_MenuSelect();

bool Overlay_IsMenuOpen();