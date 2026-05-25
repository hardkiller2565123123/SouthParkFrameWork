#include "PatchToggles.h"
#include "Config.h"
#include "Notification.h"
#include "framework.h"

static bool g_SkipIntro = false;

void PatchToggles_Init()
{
    g_SkipIntro = Config_GetBool("Patches", "SkipIntro", false);

    Framework_Log("[PATCH] Patch toggles initialized");
}

bool PatchToggles_GetSkipIntro()
{
    return g_SkipIntro;
}

void PatchToggles_ToggleSkipIntro()
{
    g_SkipIntro = !g_SkipIntro;

    Config_SetBool("Patches", "SkipIntro", g_SkipIntro);

    Notification_Push(
        g_SkipIntro
        ? "[PATCH] Skip Intro enabled"
        : "[PATCH] Skip Intro disabled"
    );

    // Real patch bytes go here later.
}