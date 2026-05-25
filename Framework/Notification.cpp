#include "Notification.h"
#include "framework.h"

#include <windows.h>
#include <vector>
#include <string>

struct NotificationItem
{
    std::string Text;
    DWORD StartTime;
};

static std::vector<NotificationItem> g_Notifications;

void Notification_Init()
{
    Framework_Log("[NOTIFY] Notification system initialized");
}

void Notification_Push(const char* text)
{
    if (!text)
        return;

    NotificationItem item;
    item.Text = text;
    item.StartTime = GetTickCount();

    g_Notifications.push_back(item);

    Framework_Log(text);
}

static void DrawNotificationText(
    ID3DXFont* font,
    const char* text,
    int x,
    int y
)
{
    if (!font || !text)
        return;

    RECT shadow;
    shadow.left = x + 1;
    shadow.top = y + 1;
    shadow.right = x + 700;
    shadow.bottom = y + 40;

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + 700;
    rect.bottom = y + 40;

    font->DrawTextA(
        NULL,
        text,
        -1,
        &shadow,
        DT_LEFT | DT_NOCLIP,
        D3DCOLOR_ARGB(220, 0, 0, 0)
    );

    font->DrawTextA(
        NULL,
        text,
        -1,
        &rect,
        DT_LEFT | DT_NOCLIP,
        D3DCOLOR_ARGB(255, 80, 255, 80)
    );
}

void Notification_Render(
    LPDIRECT3DDEVICE9 device,
    ID3DXFont* font
)
{
    if (!device || !font)
        return;

    DWORD now = GetTickCount();

    for (size_t i = 0; i < g_Notifications.size();)
    {
        if (now - g_Notifications[i].StartTime > 4000)
            g_Notifications.erase(g_Notifications.begin() + i);
        else
            i++;
    }

    D3DVIEWPORT9 vp;

    if (FAILED(device->GetViewport(&vp)))
        return;

    int x = 20;
    int y = (int)vp.Height - 120;

    for (size_t i = 0; i < g_Notifications.size(); i++)
    {
        DrawNotificationText(
            font,
            g_Notifications[i].Text.c_str(),
            x,
            y + ((int)i * 22)
        );
    }
}