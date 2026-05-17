#include "pch.h"
#include "TrayIconService.h"
#include "Global.h"

bool TrayIconService::Initialize(HWND hwnd,
    HICON icon,
    const std::wstring& tooltip)
{
    m_hwnd = hwnd;

    m_nid = {};

    m_nid.cbSize = sizeof(NOTIFYICONDATAW);

    m_nid.hWnd = hwnd;

    m_nid.uID = 1;

    m_nid.uFlags =
        NIF_MESSAGE |
        NIF_ICON |
        NIF_TIP;

    m_nid.uCallbackMessage = WM_TRAYICON;

    m_nid.hIcon = icon;

    wcscpy_s(m_nid.szTip, tooltip.c_str());

    if (!SetWindowSubclass(
        hwnd,
        SubclassProc,
        1,
        reinterpret_cast<DWORD_PTR>(this)))
    {
        return false;
    }

    return true;
}

void TrayIconService::Show()
{
    if (m_visible)
        return;

    Shell_NotifyIconW(NIM_ADD, &m_nid);

    m_visible = true;
}

void TrayIconService::Hide()
{
    if (!m_visible)
        return;

    Shell_NotifyIconW(NIM_DELETE, &m_nid);

    m_visible = false;
}

void TrayIconService::Cleanup()
{
    Hide();

    if (m_hwnd)
    {
        RemoveWindowSubclass(
            m_hwnd,
            SubclassProc,
            1);
    }
}

void TrayIconService::UpdateTooltip(const std::wstring& text)
{
    wcscpy_s(m_nid.szTip, text.c_str());

    m_nid.uFlags = NIF_TIP;

    Shell_NotifyIconW(NIM_MODIFY, &m_nid);

    m_nid.uFlags =
        NIF_MESSAGE |
        NIF_ICON |
        NIF_TIP;
}

void TrayIconService::HandleTrayMessage(LPARAM lParam)
{
    switch (LOWORD(lParam))
    {
    case WM_LBUTTONUP:
    {
        LeftClickEvent.Invoke();
        break;
    }

    case WM_RBUTTONUP:
    {
        ShowContextMenu();
        break;
    }

    case WM_LBUTTONDBLCLK:
    {
        LeftClickEvent.Invoke();
        break;
    }

    default:
        break;
    }
}

LRESULT CALLBACK TrayIconService::SubclassProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData)
{
    auto self =
        reinterpret_cast<TrayIconService*>(dwRefData);

    if (!self)
    {
        return DefSubclassProc(
            hwnd,
            msg,
            wParam,
            lParam);
    }

    switch (msg)
    {
    case WM_TRAYICON:
    {
        self->HandleTrayMessage(lParam);
        return 0;
    }

    case WM_DESTROY:
    {
        self->Cleanup();
        break;
    }

    default:
        break;
    }

    return DefSubclassProc(
        hwnd,
        msg,
        wParam,
        lParam);
}

void TrayIconService::ShowContextMenu()
{
    HMENU menu = CreatePopupMenu();

    AppendMenuW(
        menu,
        MF_STRING,
        (UINT)TrayMenuId::ShowMainWindow,
        L"显示主窗口");

    AppendMenuW(
        menu,
        MF_STRING,
        (UINT)TrayMenuId::ShowControlWindow,
        L"显示控制窗口");

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    AppendMenuW(
        menu,
        MF_STRING,
        (UINT)TrayMenuId::Settings,
        L"设置");

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    AppendMenuW(
        menu,
        MF_STRING,
        (UINT)TrayMenuId::Exit,
        L"退出");

    POINT pt{};

    GetCursorPos(&pt);

    SetForegroundWindow(m_hwnd);

    UINT cmd = TrackPopupMenu(
        menu,
        TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x,
        pt.y,
        0,
        m_hwnd,
        nullptr);

    DestroyMenu(menu);

    switch ((TrayMenuId)cmd)
    {
    case TrayMenuId::ShowMainWindow:
        ShowMainWindowEvent.Invoke();
        break;

    case TrayMenuId::ShowControlWindow:
        ShowControlWindowEvent.Invoke();
        break;

    case TrayMenuId::Settings:
        SettingsEvent.Invoke();
        break;

    case TrayMenuId::Exit:
        g_processExitEvent.Invoke();
        break;

    default:
        break;
    }
}
