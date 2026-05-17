#include "PipeService.h"
#include <mutex>
#include "Logger.h"
#include <cstring>
#include <sddl.h>
#include "WorkerQueue.h"
#include "Appdata.h"

#pragma comment(lib, "advapi32.lib")


PipeService::~PipeService()
{
    Stop();
}

void PipeService::Stop()
{
    if (!m_running) return;
    m_running = false; // 1. 标记退出状态

    // 2. 强行取消 m_pipeRead 上挂起的阻塞 ReadFile 操作
    if (m_pipeRead != INVALID_HANDLE_VALUE) {
        ::CancelIoEx(m_pipeRead, nullptr);
    }

    // 3. 等待读取线程安全退出
    if (m_readThread.joinable()) {
        m_readThread.join();
    }

    // 4. 此时 ReadFile 已退出，句柄锁已释放，可以安全 CloseHandle
    if (m_pipeRead != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipeRead);
        m_pipeRead = INVALID_HANDLE_VALUE;
    }

    // 5. 同样的逻辑处理写管道
    if (m_pipeWrite != INVALID_HANDLE_VALUE) {
        // 如果是服务端创建的管道，使用 DisconnectNamedPipe 强行断开
        ::DisconnectNamedPipe(m_pipeWrite);
        CloseHandle(m_pipeWrite);
        m_pipeWrite = INVALID_HANDLE_VALUE;
    }
    if (m_writeThread.joinable()) {
        m_writeThread.join();
    }

    WorkerQueue<PipeMessage>::Instance().Stop();
}

void PipeService::StartAsServer()
{
    if (m_running) return;
    m_running = true;

    // A写 → A2B
    m_pipeWrite = CreatePipeServer(PIPE_A2B, PIPE_ACCESS_OUTBOUND);

    // A读 ← B2A
    m_pipeRead = ConnectPipeClient(PIPE_B2A, GENERIC_READ);

    m_readThread = std::thread(&PipeService::ReadLoop, this);
    m_writeThread = std::thread(&PipeService::WriteLoop, this);
    WorkerQueue<PipeMessage>::Instance().Start();
}

void PipeService::StartAsClient()
{
    if (m_running) return;
    m_running = true;

    // B读 ← A2B
    m_pipeRead = ConnectPipeClient(PIPE_A2B, GENERIC_READ);

    // B写 → B2A
    m_pipeWrite = CreatePipeServer(PIPE_B2A, PIPE_ACCESS_OUTBOUND);

    m_readThread = std::thread(&PipeService::ReadLoop, this);
    m_writeThread = std::thread(&PipeService::WriteLoop, this);
    WorkerQueue<PipeMessage>::Instance().Start();
}

HANDLE PipeService::CreatePipeServer(const wchar_t* name, DWORD access)
{
    HANDLE pipe = CreateNamedPipeW(
        name,
        access,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        4096, 4096,
        0,
        nullptr);

    if (pipe == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    BOOL ok = ConnectNamedPipe(pipe, nullptr) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (!ok)
    {
        CloseHandle(pipe);
        return INVALID_HANDLE_VALUE;
    }

    return pipe;
}

HANDLE PipeService::ConnectPipeClient(const wchar_t* name, DWORD access)
{
    while (m_running)
    {
        if (!WaitNamedPipeW(name, 200))
        {
            Sleep(100);
            continue;
        }

        HANDLE pipe = CreateFileW(
            name,
            access,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (pipe != INVALID_HANDLE_VALUE)
        {
            DWORD mode = PIPE_READMODE_MESSAGE;
            SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr);
            return pipe;
        }
    }

    return INVALID_HANDLE_VALUE;
}

void PipeService::ReadLoop()
{
    uint8_t buffer[4096];

    while (m_running)
    {
        DWORD readBytes = 0;

        BOOL ok = ReadFile(
            m_pipeRead,
            buffer,
            sizeof(buffer),
            &readBytes,
            nullptr);

        if (!ok)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                break;
            continue;
        }

        HandleBuffer(buffer, readBytes);
    }
}

void PipeService::HandleBuffer(uint8_t* buffer, DWORD readBytes)
{
    uint8_t* ptr = buffer;
    uint8_t* end = buffer + readBytes;

    while (ptr + sizeof(IPCHeader) <= end)
    {
        IPCHeader header;
        memcpy(&header, ptr, sizeof(header));

        if (header.magic != 0xA0A1)
        {
            LOG_ERROR << "Invalid packet";
            break;
        }

        uint8_t* payload = ptr + sizeof(header);
        uint8_t* next = payload + header.size;

        if (next > end)
            break;

        switch (header.category)
        {
        case MsgCategory::FilterRule:
        {
            HWND hwnd;
            memcpy(&hwnd, payload, sizeof(HWND));

            if (m_filterRuleHandler)
                m_filterRuleHandler(hwnd);
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

        case MsgCategory::HotkeyEdit:
        {
            auto* data = (HotkeyEditPayload*)payload;

            Key key{ (ModifierMask)data->modifiers, data->vk };

            if (m_hotkeyEditHandler)
                m_hotkeyEditHandler(data->action, key);
            break;
        }

        case MsgCategory::InputMethod:
        {
            uint32_t type;
            memcpy(&type, payload, sizeof(uint32_t));

            if (m_inputMethodHandler)
                m_inputMethodHandler((InputType)type);
            break;
        }
        }

        ptr = next;
    }
}

void PipeService::SendFilterRule(HWND hwnd)
{
    WorkerQueue<PipeMessage>::Instance().Push({ MsgCategory::FilterRule, hwnd });
}

void PipeService::SendHotkeyMsg(UINT msg)
{
    WorkerQueue<PipeMessage>::Instance().Push({ MsgCategory::HotkeyMsg, msg });
}

void PipeService::SendInputMethod(uint32_t type)
{
    WorkerQueue<PipeMessage>::Instance().Push({ MsgCategory::InputMethod, type });
}

void PipeService::SendHotkeyEdit(UINT action, const Key& key)
{
    HotkeyEditPayload payload{ action, (uint8_t)key.modifiers, key.vk };
    WorkerQueue<PipeMessage>::Instance().Push({ MsgCategory::HotkeyEdit, payload });
}

void PipeService::WriteLoop()
{
    WorkerQueue<PipeMessage>::Instance().SetHandler(
        [this](const PipeMessage& msg)
        {
            std::visit([&](auto&& arg)
                {
                    if (m_pipeWrite == INVALID_HANDLE_VALUE) return;

                    auto result = WritePacket(
                        m_pipeWrite,
                        msg.category,
                        arg);

                    if (!result.success)
                    {
                        LOG_ERROR << "Write failed: " << result.error;
                    }

                }, msg.data);
        });
}