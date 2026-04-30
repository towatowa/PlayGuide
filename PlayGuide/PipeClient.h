#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <string>
#include "Event.h"
#include "Windows.h"
#include "Appdata.h"

class PipeClient
{
public:
    static PipeClient& Get();

    winrt::fire_and_forget StartAsync() noexcept;
    void Stop() noexcept;
    bool isRunning() noexcept { return m_running; }

    Event<SimpleEvent> handler;

private:
    PipeClient() = default;
   ~PipeClient() { Stop(); }
    void Worker();
    void ReadLoop(HANDLE pipe);
   
private:
    std::atomic<bool> m_running{ false };
    std::atomic<bool> m_stopRequested{ false };
};
