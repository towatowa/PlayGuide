#include "PipeService.h"
#include <mutex>
#include "Logger.h"

void PipeService::Start()
{
    if (m_running) return;

    m_running = true;
    m_thread = std::thread(&PipeService::ServerLoop, this);
    m_thread.detach();
}

void PipeService::Stop()
{
    m_running = false;

    if (m_pipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
    }

    if (m_thread.joinable())
        m_thread.join();
}

void PipeService::ServerLoop()
{
    while (m_running)
    {
        m_pipe = CreateNamedPipeW(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4096,
            4096,
            0,
            nullptr);

        if (m_pipe == INVALID_HANDLE_VALUE)
            continue;

        if (!::ConnectNamedPipe(m_pipe, nullptr))
        {
            ::CloseHandle(m_pipe);
            m_pipe = INVALID_HANDLE_VALUE;
            continue;
        }

        HandleClient();

        ::CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
    }
}

void PipeService::HandleClient()
{
    uint8_t buffer[4096];

    while (m_running)
    {
        DWORD read = 0;
        BOOL ok = ::ReadFile(m_pipe, buffer, sizeof(buffer), &read, nullptr);

        if (!ok)
        {
            DWORD err = GetLastError();

            if (err == ERROR_BROKEN_PIPE)
                break;

            continue; // ⭐ 不要退出
        }

        IPCHeader* header = reinterpret_cast<IPCHeader*>(buffer);
        uint8_t* payload = buffer + sizeof(IPCHeader);

        switch (header->category)
        {
        case MsgCategory::InputMethod:
        {
            uint32_t imeType;
            memcpy(&imeType, payload, sizeof(uint32_t));

            if (m_inputMethodHandler)
                m_inputMethodHandler(static_cast<InputType>(imeType));
            break;
        }

        case MsgCategory::FilterRule:
        {
            HWND hwnd;
            memcpy(&hwnd, payload, sizeof(HWND));

            if (m_filterRuleHandler)
                m_filterRuleHandler(hwnd);
            break;
        }

        case MsgCategory::HotkeyEdit:
        {
            UINT Msg;
            uint8_t mod;
            USHORT vk;

            memcpy(&Msg, payload, sizeof(UINT));
            memcpy(&mod, payload + sizeof(UINT), sizeof(uint8_t));
            memcpy(&vk, payload + sizeof(UINT) + sizeof(uint8_t), sizeof(USHORT));

            Key key{ static_cast<ModifierMask>(mod), vk };
            if (m_hotkeyEditHandler)
                m_hotkeyEditHandler(Msg, key);
            break;
        }
        case MsgCategory::HotkeyMsg:
        {
            UINT msg;
            memcpy(&msg, payload, sizeof(UINT));
            if (m_hotkeyMsgHandler)
                m_hotkeyMsgHandler(msg);
            break;
        }
        default:
            break;
        }
    }
}

template<typename T>
struct WriteResult
{
    bool success;
    DWORD written;
    DWORD error;
};

template<typename T>
WriteResult<T> WritePacket(HANDLE pipe, MsgCategory cat, const T& data)
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

    BOOL ok = WriteFile(
        pipe,
        &p,
        sizeof(p),
        &written,
        nullptr);

    if (!ok)
    {
        return {
            false,
            written,
            GetLastError()
        };
    }

    // 关键：写入字节数不完整也算失败
    if (written != sizeof(p))
    {
        return {
            false,
            written,
            ERROR_IO_INCOMPLETE
        };
    }

    return {
        true,
        written,
        0
    };
}

void PipeService::SendInputMethod(uint32_t imeType)
{
    std::lock_guard lock(m_writeMutex);

    if (m_pipe == INVALID_HANDLE_VALUE) return;

    WritePacket(m_pipe, MsgCategory::InputMethod, imeType);
}

void PipeService::SendHotkeyEdit(UINT action, const Key& key)
{
    std::lock_guard lock(m_writeMutex);

    if (m_pipe == INVALID_HANDLE_VALUE) return;

    HotkeyEditPayload payload{ action, static_cast<uint8_t>(key.modifiers), key.vk };

    WritePacket(m_pipe, MsgCategory::HotkeyEdit, payload);
}

void PipeService::SendFilterRule(HWND hwnd)
{
    std::lock_guard lock(m_writeMutex);

    if (m_pipe == INVALID_HANDLE_VALUE) return;

    WritePacket(m_pipe, MsgCategory::FilterRule, hwnd);
}

void PipeService::SendHotkeyMsg(UINT msg)
{
    std::lock_guard lock(m_writeMutex);

    if (m_pipe == INVALID_HANDLE_VALUE) return;
    
    //WritePacket(m_pipe, MsgCategory::HotkeyMsg, msg);
    auto result = WritePacket(m_pipe, MsgCategory::HotkeyMsg, msg);

    if (!result.success)
    {
        LOG_ERROR << "Pipe write failed, err=" << result.error
            << ", written=" << result.written;
    }
}
