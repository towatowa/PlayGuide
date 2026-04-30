#pragma once
#include <cstdint>
#include <utility>
#include <vector>
#include <functional>
#include <mutex>

template <typename... Args>
struct Event
{
public:
    using Handler = std::function<void(Args...)>;
    using Token = uint32_t;

    struct EventRevoker
    {
        Event* event = nullptr;
        Token token = 0;

        EventRevoker() = default;

        EventRevoker(Event* e, Token t)
            : event(e), token(t)
        {
        }

        EventRevoker(const EventRevoker&) = delete;

        EventRevoker(EventRevoker&& other) noexcept
        {
            *this = std::move(other);
        }

        EventRevoker& operator=(EventRevoker&& other) noexcept
        {
            if (this != &other)
            {
                Reset();
                event = other.event;
                token = other.token;
                other.event = nullptr;
            }
            return *this;
        }

        ~EventRevoker()
        {
            Reset();
        }

        void Reset()
        {
            if (event)
            {
                event->Unsubscribe(token);
                event = nullptr;
            }
        }
    };

    using  AutoRevokeT = winrt::auto_revoke_t;

public:
    // =========================
    // Subscribe
    // =========================
    Token operator()(Handler h)
    {
        std::lock_guard lock(m_mutex);

        Token t = ++m_next;
        m_handlers.emplace_back(t, std::move(h));

        return t;
    }

    // =========================
    // Unsubscribe
    // =========================
    void operator()(Token token)
    {
        std::lock_guard lock(m_mutex);

        auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
            [token](auto& p) { return p.first == token; });

        if (it != m_handlers.end())
            m_handlers.erase(it);
    }

    // =========================
    // RAII subscribe
    // =========================
    EventRevoker operator()(AutoRevokeT, Handler h)
    {
        Token t = (*this)(std::move(h));
        return EventRevoker(this, t);
    }

    // =========================
    // once (只触发一次)
    // =========================
    EventRevoker Once(Handler h)
    {
        std::shared_ptr<bool> alive = std::make_shared<bool>(true);

        Handler wrapper = [this, h = std::move(h), alive](Args... args)
            {
                if (!*alive) return;

                *alive = false;
                h(args...);
            };

        return (*this)(winrt::auto_revoke, std::move(wrapper));
    }

    // =========================
    // Invoke
    // =========================
    void Invoke(Args... args)
    {
        std::vector<std::pair<Token, Handler>> copy;

        {
            std::lock_guard lock(m_mutex);
            copy = m_handlers;
        }

        for (auto& [_, handler] : copy)
        {
            handler(args...);
        }
    }

    // =========================
    // helper
    // =========================
    void Unsubscribe(Token t)
    {
        (*this)(t);
    }

private:
    std::vector<std::pair<Token, Handler>> m_handlers;
    Token m_next = 0;
    std::mutex m_mutex;
};