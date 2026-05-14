#pragma once
#include <array>
#include <bitset>
#include <cstdint>
#include <Windows.h>
#include "utils.h"
#include <vector>
#include "../PlayGuideService/KeyEvent.h"
#include "Appdata.h"

struct KeyHistory
{
    uint64_t lastDownTick = 0;     // 最近一次 KeyDown 的 tick
    uint64_t lastUpTick = 0;     // 最近一次 KeyUp 的 tick
    uint32_t repeatCount = 0;     // 当前按压周期内的自动重复次数

    uint64_t pressId = 0;          // 物理按压实例
};

class KeyStateTracker
{
private:
    KeyStateTracker() = default;
public:
    static constexpr size_t VK_COUNT = 256;
    static constexpr size_t HOLD_THRESHOLD = 3;
    static constexpr size_t MOD_COUNT = 8;
public:
    void Reset();
    void Update(const SimpleEvent& ev);

    // ---- 基础查询 ----
    bool IsDown(USHORT vk) const;
    bool IsInjected(USHORT vk) const;
    bool IsRepeating(USHORT vk) const;

    // ---- 组合键 / 修饰键 ----
    bool Ctrl()  const;
    bool Shift() const;
    bool Alt()   const;
    bool Win()   const;

    bool HasAnyModifier() const;
    bool IsModifierOnly() const;

    bool IsTargetModifiersOnly(const uint8_t& mask) const;
    bool IsOnlyOneKeyDown(const USHORT& vk);
    // ---- 时间 / 顺序 ----
    bool IsInitialDown(USHORT vk) const;
    uint64_t LastDownTick(USHORT vk) const;
    uint64_t LastUpTick(USHORT vk) const;
    bool IsHold(USHORT vk) const;
    bool IsSamePress(USHORT vk, uint64_t id);
    // ---- 规则辅助 ----
    bool MatchChord(const std::initializer_list<USHORT>& modifiers,
        USHORT mainKey,
        const KeyEvent& ev) const;
    static KeyStateTracker* instance();
    ModifierMask GetCurrentModifiersKey() const { return static_cast<ModifierMask>(m_modMask); }
private:
    uint64_t m_tick = 0;
    uint64_t m_nextPressId{ 0 };
    std::bitset<VK_COUNT> m_down;
    std::bitset<VK_COUNT> m_injected;
    std::bitset<VK_COUNT> m_repeating;

    std::array<KeyHistory, VK_COUNT> m_history;
    uint8_t m_modMask{ 0 };
    static KeyStateTracker* s_instance;
};
