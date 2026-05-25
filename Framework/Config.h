#pragma once

void Config_Init();

bool Config_GetBool(const char* section, const char* key, bool fallback);
int Config_GetInt(const char* section, const char* key, int fallback);

void Config_SetBool(const char* section, const char* key, bool value);
void Config_SetInt(const char* section, const char* key, int value);