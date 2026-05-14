#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class WorkerQueue
{
public:
    using Handler = std::function<void(const T&)>;

    static WorkerQueue& Instance()
    {
        static WorkerQueue inst;
        return inst;
    }

    void Push(const T& v)
    {
        {
            std::lock_guard lock(m_mutex);
            m_queue.push(v);
        }
        m_cv.notify_one();
    }

    void SetHandler(Handler h)
    {
        std::lock_guard lock(m_handlerMutex);
        m_handler = std::move(h);
    }

    void Start()
    {
        m_running = true;
        m_thread = std::thread(&WorkerQueue::Run, this);
        //m_thread.detach();
    }

    void Stop()
    {
        m_running = false;
        m_cv.notify_all(); // 唤醒线程以便退出
        if (m_thread.joinable())
            m_thread.join();
    }

private:
    void Run()
    {
        while (m_running)
        {
            T value = Pop();

            Handler handler;
            {
                std::lock_guard lock(m_handlerMutex);
                handler = m_handler;
            }

            if (handler)
                handler(value);
            Sleep(50);
        }
    }

    T Pop()
    {
        std::unique_lock lock(m_mutex);
        m_cv.wait(lock, [&] { return !m_queue.empty(); });

        T v = std::move(m_queue.front());
        m_queue.pop();
        return v;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    Handler m_handler;
    std::mutex m_handlerMutex;
    std::atomic<bool> m_running{ false };
    std::thread m_thread;
};

