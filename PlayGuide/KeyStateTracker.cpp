#include "KeyStateTracker.h"

KeyStateTracker* KeyStateTracker::s_instance = nullptr;

void KeyStateTracker::Reset()
{
    m_tick = 0;
    m_down.reset();
    m_injected.reset();
    m_repeating.reset();
    m_history.fill({});
}

void KeyStateTracker::Update(const SimpleEvent& ev)
{
    ++m_tick;

    const USHORT vk = ev.vk;

    auto& h = m_history[vk];

    if (ev.action == KeyAction::KeyDown)
    {
        if (m_down[vk])
        {
            // 自动重复
            m_repeating.set(vk);
            m_history[vk].repeatCount++;
        }
        else
        {
            m_down.set(vk);
            m_repeating.reset(vk);
            m_history[vk].lastDownTick = m_tick;
            m_history[vk].repeatCount = 0;
            ++m_nextPressId;
            m_history[vk].pressId = m_nextPressId;
        }
        // 若是修饰键，设置对应位
        if (int idx = keyutils::ModifierVkToIndex(vk); idx >= 0)
            m_modMask |= (uint8_t(1) << idx);
    }
    else // KeyUp
    {
        m_down.reset(vk);
        m_repeating.reset(vk);
        m_history[vk].lastUpTick = m_tick;
        // 若是修饰键，清除对应位
        if (int idx = keyutils::ModifierVkToIndex(vk); idx >= 0)
            m_modMask &= ~uint8_t(1 << idx);
    }
}

// ----------------- 查询 -----------------

bool KeyStateTracker::IsDown(USHORT vk) const
{
    return m_down[vk];
}

bool KeyStateTracker::IsInjected(USHORT vk) const
{
    return m_injected[vk];
}

bool KeyStateTracker::IsRepeating(USHORT vk) const
{
    return m_repeating[vk];
}

bool KeyStateTracker::IsInitialDown(USHORT vk) const
{
    return m_down[vk] && !m_repeating[vk];
}

uint64_t KeyStateTracker::LastDownTick(USHORT vk) const
{
    return m_history[vk].lastDownTick;
}

uint64_t KeyStateTracker::LastUpTick(USHORT vk) const
{
    return m_history[vk].lastUpTick;
}

bool KeyStateTracker::IsHold(USHORT vk) const
{
    bool isHold = m_down[vk] &&
        (m_tick - m_history[vk].lastDownTick) > HOLD_THRESHOLD;
    return isHold;
}

bool KeyStateTracker::IsSamePress(USHORT vk, uint64_t id) {
    return m_history[vk].pressId == id;
}

// ----------------- 修饰键 -----------------

bool KeyStateTracker::Ctrl() const
{
    return m_down[VK_LCONTROL] || m_down[VK_RCONTROL];
}

bool KeyStateTracker::Shift() const
{
    return m_down[VK_LSHIFT] || m_down[VK_RSHIFT];
}

bool KeyStateTracker::Alt() const
{
    return m_down[VK_LMENU] || m_down[VK_RMENU];
}

bool KeyStateTracker::Win() const
{
    return m_down[VK_LWIN] || m_down[VK_RWIN];
}

bool KeyStateTracker::HasAnyModifier() const
{
    return Ctrl() || Shift() || Alt() || Win();
}

bool KeyStateTracker::IsModifierOnly() const
{
    for (USHORT vk = 0; vk < VK_COUNT; ++vk)
    {
        if (m_down[vk])
        {
            if (vk != VK_LCONTROL && vk != VK_RCONTROL &&
                vk != VK_LSHIFT && vk != VK_RSHIFT &&
                vk != VK_LMENU && vk != VK_RMENU &&
                vk != VK_LWIN && vk != VK_RWIN)
                return false;
        }
    }
    return true;
}

bool KeyStateTracker::IsTargetModifiersOnly(const uint8_t& mask) const
{
    return m_modMask == mask;
}

bool KeyStateTracker::IsOnlyOneKeyDown(const USHORT& vk)
{
    for (auto i = 0; i < VK_COUNT; ++i)
    {
        if (i != vk && m_down[i])
        {
            return false;
        }
    }

    return true;
}

// ----------------- 组合键匹配 -----------------

bool KeyStateTracker::MatchChord(
    const std::initializer_list<USHORT>& modifiers,
    USHORT mainKey,
    const KeyEvent& ev) const
{
    // 只能由主键的 InitialDown 触发
    if (ev.vk != mainKey || ev.action != OptType::KeyDown)
        return false;

    if (!IsInitialDown(mainKey))
        return false;

    // 所有修饰键必须已按下
    for (auto vk : modifiers)
    {
        if (!IsDown(vk))
            return false;
    }

    return true;
}

KeyStateTracker* KeyStateTracker::instance()
{
    // 使用函数局部静态实例（Meyers singleton），C++11 起线程安全
    static KeyStateTracker singletonInstance;
    // 保持 s_instance 指向同一对象，便于其他使用静态指针的代码继续工作
    s_instance = &singletonInstance;
    return s_instance;
}