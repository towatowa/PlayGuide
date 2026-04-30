#include "pch.h"
#include "PipeClient.h"
#include "Logger.h"
#include "Appdata.h"

PipeClient& PipeClient::Get()
{
    static PipeClient ins;
    return ins;
}

winrt::fire_and_forget PipeClient::StartAsync() noexcept
{
    co_await winrt::resume_background();
    if (m_running)
        co_return;
    m_running = true;
    m_stopRequested = false;
    Worker();
}

void PipeClient::Stop() noexcept
{
    m_stopRequested = true;

    // ❗ 不做 join（避免 UI 卡死）
    // 让 worker 自己退出
}
void PipeClient::Worker()
{
    while (!m_stopRequested)
    {
        HANDLE pipe = CreateFileW(
            L"\\\\.\\pipe\\KeyHookPipe",
            GENERIC_READ,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            nullptr);

        if (pipe == INVALID_HANDLE_VALUE)
        {
            Sleep(500); // ❗重连间隔
            continue;
        }

        ReadLoop(pipe);

        CloseHandle(pipe);
    }

    m_running = false;
}

void PipeClient::ReadLoop(HANDLE pipe)
{
    SimpleEvent msg{};
    DWORD read = 0;
    while (!m_stopRequested)
    {
        BOOL ok = ReadFile(pipe, &msg, sizeof(msg), &read, nullptr);

        if (!ok || read != sizeof(msg))
        {
            // ❗ pipe断开
            LOG_INFO << "Service通信已断卡\n";
            break;
        }


        handler.Invoke(msg);

        LOG_INFO << "Recv vk: " << msg.vk << "\n";
    }
}