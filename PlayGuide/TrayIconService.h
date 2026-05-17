#pragma once

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <string>

#include "Event.h"

enum class TrayMenuId : UINT
{
    ShowMainWindow = 1001,
    ShowControlWindow,
    Settings,
    Exit
};

class TrayIconService
{
public:

    static TrayIconService& Get()
    {
        static TrayIconService ins;
        return ins;
    }

public:

    bool Initialize(HWND hwnd,
        HICON icon,
        const std::wstring& tooltip);

    void Show();

    void Hide();

    void UpdateTooltip(const std::wstring& text);

    void Cleanup();

    bool IsVisible() const noexcept
    {
        return m_visible;
    }

public:

    // =========================
    // Events
    // =========================

    Event<> LeftClickEvent;

    Event<> RightClickEvent;

    Event<> DoubleClickEvent;

    Event<> ShowMainWindowEvent;

    Event<> ShowControlWindowEvent;

    Event<> SettingsEvent;

    Event<> ExitEvent;
private:

    TrayIconService() = default;

    ~TrayIconService() = default;

    TrayIconService(const TrayIconService&) = delete;

    TrayIconService& operator=(const TrayIconService&) = delete;

private:

    void HandleTrayMessage(LPARAM lParam);

    static LRESULT CALLBACK SubclassProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData);

    void ShowContextMenu();

private:

    static constexpr UINT WM_TRAYICON = WM_APP + 100;

private:

    HWND m_hwnd{};

    NOTIFYICONDATAW m_nid{};

    bool m_visible{ false };
};

