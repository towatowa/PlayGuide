#pragma once
#include <Windows.h>
#include <thread>
#include <atomic>
#include <variant>
#include "Appdata.h"
#include <functional>

using InputMethodHandler = std::function<void(InputType)>;
using FilterRuleHandler = std::function<void(HWND)>;
using HotkeyEditHandler = std::function<void(UINT, const Key&)>;
using HotkeyMsgHandler = std::function<void(UINT)>;

template<typename T>
struct WriteResult
{
    bool success;
    DWORD written;
    DWORD error;
};

class PipeService
{
public:
    static PipeService& Get()
    {
        static PipeService instance;
        return instance;
    }
    ~PipeService();

    void StartAsServer(); // UI
    void StartAsClient(); // Backend
    void Stop();

    // ===== 发送接口 =====
    void SendInputMethod(uint32_t imeType);
    void SendHotkeyEdit(UINT action, const Key& key);
    void SendFilterRule(HWND hwnd);
    void SendHotkeyMsg(UINT msg);

    void SetInputMethodHandler(InputMethodHandler handler) { m_inputMethodHandler = std::move(handler); }
    void SetFilterRuleHandler(FilterRuleHandler handler) { m_filterRuleHandler = std::move(handler); }
    void SetHotkeyEditHandler(HotkeyEditHandler handler) { m_hotkeyEditHandler = std::move(handler); }
    void SetHotkeyMsgHandler(HotkeyMsgHandler handler) { m_hotkeyMsgHandler = std::move(handler); }
    template<typename T>
    WriteResult<T> WritePacket(HANDLE pipe, MsgCategory cat, const T& data);
private:
    void ReadLoop();
    void WriteLoop();

    HANDLE CreatePipeServer(const wchar_t* name, DWORD access);
    HANDLE ConnectPipeClient(const wchar_t* name, DWORD access);

    void HandleBuffer(uint8_t* buffer, DWORD size);

    InputMethodHandler m_inputMethodHandler;
    FilterRuleHandler m_filterRuleHandler;
    HotkeyEditHandler m_hotkeyEditHandler;
    HotkeyMsgHandler m_hotkeyMsgHandler;

private:
    std::atomic<bool> m_running{ false };

    std::thread m_readThread;
    std::thread m_writeThread;

    HANDLE m_pipeRead = INVALID_HANDLE_VALUE;
    HANDLE m_pipeWrite = INVALID_HANDLE_VALUE;

    static constexpr const wchar_t* PIPE_A2B = L"\\\\.\\pipe\\PlayGuide_A2B";
    static constexpr const wchar_t* PIPE_B2A = L"\\\\.\\pipe\\PlayGuide_B2A";
};

template<typename T>
WriteResult<T> PipeService::WritePacket(HANDLE pipe, MsgCategory cat, const T& data)
{
    struct Packet
    {
        IPCHeader header;
        T payload;
    };

    Packet p;
    p.header.category = cat;
    p.header.type = 0;
    p.header.size = sizeof(T);
    p.payload = data;

    DWORD written = 0;
    BOOL ok = WriteFile(pipe, &p, sizeof(Packet), &written, nullptr);

    if (!ok)
        return { false, written, GetLastError() };

    if (written != sizeof(Packet))
        return { false, written, ERROR_IO_INCOMPLETE };

    return { true, written, 0 };
}