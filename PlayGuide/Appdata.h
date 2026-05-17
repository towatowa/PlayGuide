#pragma once
#include <Windows.h>
#include <string>
#include <unordered_map>
#include "KeyMapping.h"
#include "utils.h"
#include <variant>

enum class ModifierMask : uint8_t
{
    None = 0,
    Ctrl = 1 << 0,
    Alt = 1 << 1,
    Shift = 1 << 2,
    Win = 1 << 3
};

inline ModifierMask operator&(ModifierMask a, ModifierMask b)
{
    return static_cast<ModifierMask>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline ModifierMask operator|(ModifierMask a, ModifierMask b)
{
    return static_cast<ModifierMask>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

enum class KeyAction
{
    KeyDown = 0,
    KeyUp,
};

struct SimpleEvent
{
    USHORT vk{ 0 };
    KeyAction action{ KeyAction::KeyDown };
};

struct Key
{
    Key() = default;
    explicit Key(const std::wstring_view& key) { StringToKey(key); }
    explicit Key(ModifierMask mod, USHORT vk) : modifiers(mod), vk(vk) {};
    ModifierMask modifiers{ModifierMask::None};
    USHORT vk{ 0 };
    void StringToKey(const std::wstring_view& key)
    {
        modifiers = ModifierMask::None;
        vk = 0;

        std::vector<std::wstring> parts;
        size_t start = 0;
        size_t end = key.find(L" + ");

        while (end != std::wstring::npos)
        {
            parts.emplace_back(key.substr(start, end - start));
            start = end + 3;
            end = key.find(L" + ", start);
        }

        // 最后一个
        parts.emplace_back(key.substr(start));

        for (const auto& part : parts)
        {
            if (_wcsicmp(part.c_str(), L"Ctrl") == 0)
            {
                modifiers = modifiers | ModifierMask::Ctrl;
            }
            else if (_wcsicmp(part.c_str(), L"Alt") == 0)
            {
                modifiers = modifiers | ModifierMask::Alt;
            }
            else if (_wcsicmp(part.c_str(), L"Shift") == 0)
            {
                modifiers = modifiers | ModifierMask::Shift;
            }
            else if (_wcsicmp(part.c_str(), L"Win") == 0)
            {
                modifiers = modifiers | ModifierMask::Win;
            }
            else
            {
                // 主键（最后一个非修饰键）
                vk = KeyMapping::KeyNameToVK(utils::WstringToString(part));
            }
        }
    }
    std::wstring GetString() const
    {
        std::vector<std::wstring> parts;

        // 修饰键
        if ((modifiers & ModifierMask::Ctrl) == ModifierMask::Ctrl)
            parts.push_back(L"Ctrl");

        if ((modifiers & ModifierMask::Alt) == ModifierMask::Alt)
            parts.push_back(L"Alt");

        if ((modifiers & ModifierMask::Shift) == ModifierMask::Shift)
            parts.push_back(L"Shift");

        if ((modifiers & ModifierMask::Win) == ModifierMask::Win)
            parts.push_back(L"Win");

        // 主键
        parts.push_back(utils::StringToWstring(KeyMapping::VKToKeyName(vk)));

        // 拼接
        std::wstring result;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            result += parts[i];
            if (i != parts.size() - 1)
                result += L" + ";
        }

        return result;
    }

    bool operator==(const Key& other) const
    {
        return modifiers == other.modifiers && vk == other.vk;
    }
};

namespace std
{
    template<>
    struct hash<Key>
    {
        size_t operator()(const Key& k) const noexcept
        {
            size_t h1 = std::hash<uint8_t>()((uint8_t)k.modifiers);
            size_t h2 = std::hash<USHORT>()(k.vk);

            return h1 ^ (h2 << 1);
        }
    };
}

enum class MsgCategory : uint32_t
{
    InputMethod = 1,
    HotkeyEdit = 2,
    FilterRule = 3,
    HotkeyMsg = 4,
};

struct IPCHeader
{
    uint32_t magic = 0xA0A1;
    MsgCategory category;   // 功能分类 ⭐
    uint32_t type;          // 子类型（可选）
    uint32_t size;          // payload size
};

using InputMethodMsg = uint32_t;
using HwndMsg = HWND;

struct HotkeyEditPayload
{
    UINT action;
    uint8_t modifiers;
    USHORT vk;
};

enum class InputType
{
    KeyboardHook,
    RawInput
};

struct PipeMessage {
    MsgCategory category;
    std::variant<
        uint32_t,
        HotkeyEditPayload,
        HWND
    > data;
};

inline std::wstring g_defaultWebUrl{ L"https://www.bilibili.com/" };


constexpr UINT WM_EnableHotkeys = WM_USER + 1;
constexpr UINT WM_IncreaseOpacity = WM_USER + 2;
constexpr UINT WM_DecreaseOpacity = WM_USER + 3;
constexpr UINT WM_PlayPause = WM_USER + 4;
constexpr UINT WM_SkipForward = WM_USER + 5;
constexpr UINT WM_SkipBackward = WM_USER + 6;
constexpr UINT WM_ShowHideWindow = WM_USER + 7;
constexpr UINT WM_MaximizeWindow = WM_USER + 8;

using HotKeyMap = std::unordered_map<std::wstring, Key>;

inline std::wstring g_keys[] = 
{
        L"EnableHotkeys",
        L"IncreaseOpacity",
        L"DecreaseOpacity",
        L"PlayPause",
        L"SkipForward",
        L"SkipBackward",
        L"ShowHideWindow",
        L"MaximizeWindow"
};

inline std::unordered_map<std::wstring, std::wstring> g_hotkeyIconGlyphs =
{
    // =================（播放/窗口控制） =================

    {L"PlayPause",        L"\uE768"},  // 播放/暂停
    {L"SkipForward",      L"\uE893"},  // 快进
    {L"SkipBackward",     L"\uE892"},  // 快退

    {L"ShowHideWindow",   L"\uE8C8"},  // 显示/隐藏窗口
    // ================= 透明度/窗口效果=================
    {L"IncreaseOpacity",  L"\uE70E"},  // 增强
    {L"DecreaseOpacity",  L"\uE70D"},  // 减弱
    {L"MaximizeWindow",   L"\uE740"},  // 最大化
    // ================= 备选项 未实现=================
    {L"MinimizeWindow",   L"\uE921"},  // 最小化
    {L"CloseWindow",      L"\uE8BB"},  // 关闭窗口
    {L"EnableHotkeys",    L"\uE765"},  // 启用/禁用快捷键
    // ================= 常用设置/交互 =================

    {L"Settings",         L"\uE713"},  // 设置（齿轮，UI必备）
    {L"Search",           L"\uE721"},  // 搜索（全局功能常见）
    {L"Refresh",          L"\uE72C"},  // 刷新
    {L"Back",             L"\uE72B"},  // 返回
    {L"Forward",          L"\uE72A"},  // 前进

    // ================= 音量控制 =================

    {L"VolumeUp",         L"\uE995"},  // 音量+
    {L"VolumeDown",       L"\uE992"},  // 音量-
    {L"Mute",             L"\uE74F"},  // 静音

    // ================= 扩展/备用功能 =================

    {L"FullScreen",       L"\uE740"}   // 全屏（复用最大化 glyph）
};

inline HotKeyMap g_defaultHotkeys
{
    {L"EnableHotkeys"       , Key(ModifierMask::Ctrl|ModifierMask::Alt, 'H')},//Ctrl+Alt+H
    {L"IncreaseOpacity"     , Key(ModifierMask::None, 56)},
    {L"DecreaseOpacity"     , Key(ModifierMask::None, 55)},
    {L"PlayPause"           , Key(ModifierMask::None, 192)},
    {L"SkipForward"         , Key(ModifierMask::None, 54)},
    {L"SkipBackward"        , Key(ModifierMask::None, 53)},
    {L"ShowHideWindow"      , Key(ModifierMask::None, 57)},
    {L"MaximizeWindow"      , Key(ModifierMask::None, 49)}
};

enum class LocaleTheme 
{  
    System = 0,
    Dark,
    Light,
};

enum class LocaleLanguage
{
    System,
    Chinese,
    English
};

struct HotkeyItem
{
    std::wstring name;
    std::wstring description;
    std::wstring key;
    std::wstring iconGlyph;
};

struct AppSettings
{
    LocaleTheme theme{ LocaleTheme::System };
    LocaleLanguage language{ LocaleLanguage::System };

    bool autoStart{ false };
    bool systemTrayExecute{ true };
    bool adminRunning{ false };
    bool intelCpuUseECore{ true };
    InputType inputType{ InputType::KeyboardHook };
};

struct TabInfo
{
    uint32_t idx{ 0 };
    std::wstring title{ L"" };
    std::wstring url{ L"" };
};

enum class WindowState
{
    Normal = 0,
    Minimized,
    Maximized,
    Hidden,   // 隐藏
    SystemTray //托盘隐藏
};

struct WindowData
{
    int x{ 100 };
    int y{ 100 };
    int width{ 1280 };
    int height{ 720 };
    int alpha{ 255 };//透明度
    WindowState windowState{ WindowState::Normal };
    virtual ~WindowData() = default; // ⭐关键
};



struct MainWindowData : WindowData
{
    std::wstring url;
};

struct ControlWindowData : WindowData
{
};