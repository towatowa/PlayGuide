#pragma once
#include <Windows.h>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

class IpcEventBus
{
public:
    static IpcEventBus& Instance()
    {
        static IpcEventBus inst;
        return inst;
    }

    void Start();
    void Stop();

    // 发送事件（Hook / UI / 任何线程都可调用）
    void Post(UINT msg);

    // 设置回调（UI线程消费）
    void SetHandler(std::function<void(UINT)> handler);

private:
    IpcEventBus() = default;
    ~IpcEventBus() = default;

    IpcEventBus(const IpcEventBus&) = delete;
    IpcEventBus& operator=(const IpcEventBus&) = delete;

private:
    void Worker();

private:
    std::atomic<bool> m_running{ false };
    std::thread m_thread;

    std::mutex m_mutex;
    std::queue<UINT> m_queue;

    std::function<void(UINT)> m_handler;
};
