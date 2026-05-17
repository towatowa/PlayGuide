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
        m_handler = std::move(h);
    }

    void Start()
    {
        m_running = true;
        m_thread = std::thread(&WorkerQueue::Run, this);
        m_thread.detach();
    }

    void Stop()
    {
        m_running = false;
        m_cv.notify_all(); // 唤醒线程以便退出
        if (m_thread.joinable())
            m_thread.join();
        LOG_INFO << "Stop WorkerQueue successfull.\n";
    }

private:
    void Run()
    {
        while (m_running || !m_queue.empty())
        {
            auto value = Pop();
            if (!value.has_value())
            {
                continue; // 说明是因为退出而被唤醒，直接进入下一次循环触发 while 终止
            }

            if (m_handler)
                m_handler(value.value());

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::optional<T> Pop()
    {
        std::unique_lock lock(m_mutex);
        // 队列不为空，或者程序已经停止运行，才唤醒
        m_cv.wait(lock, [&] { return !m_queue.empty() || !m_running; });
        // 如果被唤醒是因为程序停止，且队列里已经没数据了，直接返回空
        if (!m_running && m_queue.empty())
        {
            return std::nullopt;
        }
        T v = std::move(m_queue.front());
        m_queue.pop();
        return v;
    }
   
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;

    Handler m_handler;
    std::atomic<bool> m_running{ false };
    std::thread m_thread;
};

