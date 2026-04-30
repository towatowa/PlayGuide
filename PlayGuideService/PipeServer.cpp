#include "PipeServer.h"

void PipeServer::Start()
{
    if (m_running)
        return;

    m_running = true;
    m_thread = std::thread(&PipeServer::Worker, this);
}

void PipeServer::Stop()
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

void PipeServer::Worker()
{
    while (m_running)
    {
        m_pipe = CreateNamedPipeW(
            L"\\\\.\\pipe\\KeyHookPipe",
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            1024, 1024,
            0,
            nullptr);

        if (m_pipe == INVALID_HANDLE_VALUE)
            continue;

        ConnectNamedPipe(m_pipe, nullptr);

        // 等待 UI 连接
        while (m_running)
        {
            Sleep(10);
        }

        CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
    }
}

void PipeServer::Post(SimpleEvent msg)
{
    if (m_pipe == INVALID_HANDLE_VALUE)
        return;

    DWORD written = 0;
    //char buffer[32];
    //sprintf_s(buffer, "K:%u\n", msg);

    //WriteFile(m_pipe, buffer, (DWORD)strlen(buffer), &written, nullptr);
    WriteFile(m_pipe, &msg, sizeof(msg), &written, nullptr);
}
