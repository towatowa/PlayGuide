#pragma once
#include "pch.h"
#include <Windows.h>
#include <functional>
#include <mutex>
#include "Event.h"

class WinEventMonitor
{
public:
    using ForegroundCallback = std::function<void(HWND hwnd)>;

    static WinEventMonitor& Instance()
    {
        static WinEventMonitor instance;
        return instance;
    }

    // 禁止拷贝
    WinEventMonitor(const WinEventMonitor&) = delete;
    WinEventMonitor& operator=(const WinEventMonitor&) = delete;

    // 启动监听
    winrt::fire_and_forget Start()
    {
        winrt::resume_background();
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_hook)
            co_return;

        m_hook = SetWinEventHook(
            EVENT_SYSTEM_FOREGROUND,
            EVENT_SYSTEM_FOREGROUND,
            nullptr,
            WinEventMonitor::WinEventProc,
            0,
            0,
            WINEVENT_OUTOFCONTEXT
        );
    }

    // 停止监听
    void Stop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_hook)
        {
            UnhookWinEvent(m_hook);
            m_hook = nullptr;
        }
    }

    Event<HWND> ForgroundEvent;
private:
    WinEventMonitor() = default;
    ~WinEventMonitor()
    {
        Stop();
    }

    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD idEventThread,
        DWORD dwmsEventTime)
    {
        if (event == EVENT_SYSTEM_FOREGROUND)
        {
            Instance().OnForegroundChanged(hwnd);
        }
    }

    void OnForegroundChanged(HWND hwnd)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        ForgroundEvent.Invoke(hwnd);
    }

    

private:
    HWINEVENTHOOK m_hook = nullptr;
    //ForegroundCallback m_foregroundCallback;
    std::mutex m_mutex;
};
