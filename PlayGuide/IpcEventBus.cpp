#include "pch.h"
#include "IpcEventBus.h"

void IpcEventBus::Start()
{
    if (m_running)
        return;

    m_running = true;
    m_thread = std::thread(&IpcEventBus::Worker, this);
}

void IpcEventBus::Stop()
{
    m_running = false;

    if (m_thread.joinable())
        m_thread.join();
}

void IpcEventBus::Post(UINT msg)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(msg);
    }
}

void IpcEventBus::SetHandler(std::function<void(UINT)> handler)
{
    m_handler = std::move(handler);
}

void IpcEventBus::Worker()
{
    while (m_running)
    {
        UINT msg = 0;
        bool has = false;

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (!m_queue.empty())
            {
                msg = m_queue.front();
                m_queue.pop();
                has = true;
            }
        }

        if (has && m_handler)
        {
            m_handler(msg);
        }
        else
        {
            Sleep(1);
        }
    }
}
