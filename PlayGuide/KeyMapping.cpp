#include "KeyMapping.h"

namespace KeyMapping
{
    static const std::unordered_map<std::string, USHORT> s_keyNameToVK = {

        // ===== 字母 =====
        {"A", 'A'}, {"B", 'B'}, {"C", 'C'}, {"D", 'D'}, {"E", 'E'},
        {"F", 'F'}, {"G", 'G'}, {"H", 'H'}, {"I", 'I'}, {"J", 'J'},
        {"K", 'K'}, {"L", 'L'}, {"M", 'M'}, {"N", 'N'}, {"O", 'O'},
        {"P", 'P'}, {"Q", 'Q'}, {"R", 'R'}, {"S", 'S'}, {"T", 'T'},
        {"U", 'U'}, {"V", 'V'}, {"W", 'W'}, {"X", 'X'}, {"Y", 'Y'},
        {"Z", 'Z'},

        // ===== 数字（主键盘）=====
        {"0", '0'}, {"1", 49}, {"2", '2'}, {"3", '3'}, {"4", '4'},
        {"5", '5'}, {"6", '6'}, {"7", '7'}, {"8", '8'}, {"9", '9'},

        // ===== 功能键 =====
        {"F1", VK_F1},   {"F2", VK_F2},   {"F3", VK_F3},   {"F4", VK_F4},
        {"F5", VK_F5},   {"F6", VK_F6},   {"F7", VK_F7},   {"F8", VK_F8},
        {"F9", VK_F9},   {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
        {"F13", VK_F13}, {"F14", VK_F14}, {"F15", VK_F15}, {"F16", VK_F16},
        {"F17", VK_F17}, {"F18", VK_F18}, {"F19", VK_F19}, {"F20", VK_F20},
        {"F21", VK_F21}, {"F22", VK_F22}, {"F23", VK_F23}, {"F24", VK_F24},

        // ===== 控制键 =====
        {"Enter", VK_RETURN},
        {"Escape", VK_ESCAPE},
        {"Tab", VK_TAB},
        {"Backspace", VK_BACK},
        {"Space", VK_SPACE},

        // ===== 修饰键 =====
        {"L_Shift", VK_LSHIFT},
        {"R_Shift", VK_RSHIFT},
        {"L_Ctrl", VK_LCONTROL},
        {"R_Ctrl", VK_RCONTROL},
        {"L_Alt", VK_LMENU},
        {"R_Alt", VK_RMENU},
        {"Win", VK_LWIN},
        {"R_Win", VK_RWIN},
        {"Ctrl", VK_CONTROL},
        {"Alt", VK_MENU},
        {"Shift", VK_SHIFT},
        

        // ===== 锁定键 =====
        {"Caps_Lock", VK_CAPITAL},
        {"Num_Lock", VK_NUMLOCK},
        {"Scroll_Lock", VK_SCROLL},
        {"Pause", VK_PAUSE},
        // ===== 方向 / 导航 =====
        {"Up", VK_UP},
        {"Down", VK_DOWN},
        {"Left", VK_LEFT},
        {"Right", VK_RIGHT},
        {"Home", VK_HOME},
        {"End", VK_END},
        {"Page_Up", VK_PRIOR},
        {"Page_Down", VK_NEXT},

        // ===== 编辑键 =====
        {"Insert", VK_INSERT},
        {"Delete", VK_DELETE},

        // ===== 小键盘 =====
        {"Num_0", VK_NUMPAD0},
        {"Num_1", VK_NUMPAD1},
        {"Num_2", VK_NUMPAD2},
        {"Num_3", VK_NUMPAD3},
        {"Num_4", VK_NUMPAD4},
        {"Num_5", VK_NUMPAD5},
        {"Num_6", VK_NUMPAD6},
        {"Num_7", VK_NUMPAD7},
        {"Num_8", VK_NUMPAD8},
        {"Num_9", VK_NUMPAD9},

        {"Num_Add", VK_ADD},
        {"Num_Subtract", VK_SUBTRACT},
        {"Num_Multiply", VK_MULTIPLY},
        {"Num_Divide", VK_DIVIDE},
        {"Num_Decimal", VK_DECIMAL},
        {"Num_Enter", VK_RETURN}, // 注意：区分需结合 ScanCode

        // ===== 符号键（美式键盘）=====
        {"Minus", VK_OEM_MINUS},          // -
        {"Equal", VK_OEM_PLUS},           // =
        {"Left_Bracket", VK_OEM_4},       // [
        {"Right_Bracket", VK_OEM_6},      // ]
        {"Backslash", VK_OEM_5},          // \
            {"Semicolon", VK_OEM_1},          // ;
            {"Apostrophe", VK_OEM_7},         // '
            {"Comma", VK_OEM_COMMA},          // ,
            {"Period", VK_OEM_PERIOD},        // .
            {"Slash", VK_OEM_2},              // /
            {"Grave", VK_OEM_3},              // `
    };

    static const std::unordered_map<USHORT, std::string> s_vkToKeyName = [] {
        std::unordered_map<USHORT, std::string> m;
        for (const auto& kv : s_keyNameToVK)
            m[kv.second] = kv.first;
        return m;
        }();

    // ===== API =====

    USHORT KeyNameToVK(const std::string& name)
    {
        auto it = s_keyNameToVK.find(name);
        return it != s_keyNameToVK.end() ? it->second : 0;
    }

    std::string VKToKeyName(USHORT vk)
    {
        auto it = s_vkToKeyName.find(vk);
        return it != s_vkToKeyName.end() ? it->second : "UNKNOWN";
    }

    const std::unordered_map<std::string, USHORT>& GetKeyNameToVKMap()
    {
        return s_keyNameToVK;
    }

    const std::unordered_map<USHORT, std::string>& GetVKToKeyNameMap()
    {
        return s_vkToKeyName;
    }
}
