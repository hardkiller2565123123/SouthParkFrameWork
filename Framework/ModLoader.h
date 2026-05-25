#pragma once

void ModLoader_Init();
void ModLoader_LoadAll();
void ModLoader_UnloadAll();
void ModLoader_Reload();

int ModLoader_GetLoadedCount();
int ModLoader_GetDetectedCount();

const char* ModLoader_GetModName(int index);
bool ModLoader_IsModEnabled(int index);
void ModLoader_ToggleMod(int index);