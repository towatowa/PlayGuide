#pragma once
#include <Windows.h>
#include <thread>
#include <atomic>
#include <string>
#include "Appdata.h"

class PipeServer
{
public:
    static PipeServer& Get() {
        static PipeServer instance;
        return instance;
    }
    void Start();
    void Stop();

    void Post(SimpleEvent msg);

private:
    void Worker();
    PipeServer() = default;
    PipeServer(const PipeServer&) = delete;
    PipeServer& operator=(const PipeServer&) = delete;
    ~PipeServer() { Stop(); }

private:
    std::atomic<bool> m_running{ false };
    std::thread m_thread;

    HANDLE m_pipe = INVALID_HANDLE_VALUE;
};