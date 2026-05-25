#pragma once

#include <d3d9.h>
#include <d3dx9.h>

void Notification_Init();
void Notification_Push(const char* text);
void Notification_Render(LPDIRECT3DDEVICE9 device, ID3DXFont* font);