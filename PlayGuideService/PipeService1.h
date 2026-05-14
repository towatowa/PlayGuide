#pragma once
#include <Windows.h>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include "Appdata.h"
#include <mutex>

class PipeService
{
public:
    static PipeService& Get()
    {
        static PipeService instance;
        return instance;
    }

    void Start();
    void Stop();

    // ===== 发送接口 =====
    void SendInputMethod(uint32_t imeType);
    void SendHotkeyEdit(UINT action, const Key& key);
    void SendFilterRule(HWND hwnd);
    void SendHotkeyMsg(UINT msg);
    //注册回调
    void RegisterHotkeyEditHandler(std::function<void(UINT, Key)> handler) { m_hotkeyEditHandler = std::move(handler); }
    void RegisterInputMethodHandler(std::function<void(InputType)> handler) { m_inputMethodHandler = std::move(handler); }
    void RegisterFilterRuleHandler(std::function<void(HWND)> handler) { m_filterRuleHandler = std::move(handler); }
    void RegisterHotkeyMsgHandler(std::function<void(UINT)> handler) { m_hotkeyMsgHandler = std::move(handler); }

private:
    void ServerLoop();
    void HandleClient();
private:
    std::atomic<bool> m_running{ false };
    std::thread m_thread;
    HANDLE m_pipe = INVALID_HANDLE_VALUE;

    std::mutex m_writeMutex;

    static constexpr const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\PlayGuidePipe";
    std::function<void(UINT, Key)> m_hotkeyEditHandler{ nullptr };
    std::function<void(InputType)> m_inputMethodHandler{ nullptr };
    std::function<void(HWND)> m_filterRuleHandler{ nullptr };
    std::function<void(UINT)> m_hotkeyMsgHandler{ nullptr };
};