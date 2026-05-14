#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>

namespace utils
{
	static std::filesystem::path GetExeDir()
	{
		wchar_t path[MAX_PATH]{};
		GetModuleFileNameW(nullptr, path, MAX_PATH);

		return std::filesystem::path(path).parent_path();
	}

    static std::wstring StringToWstring(const std::string& s)
    {
        if (s.empty())
            return {};

        int sizeNeeded = MultiByteToWideChar(
            CP_UTF8,            // 输入是 UTF-8
            0,
            s.data(),
            (int)s.size(),
            nullptr,
            0
        );

        if (sizeNeeded <= 0)
            return {};

        std::wstring result(sizeNeeded, 0);

        MultiByteToWideChar(
            CP_UTF8,
            0,
            s.data(),
            (int)s.size(),
            result.data(),
            sizeNeeded
        );

        return result;
    }

    static inline std::string WstringToString(const std::wstring& ws)
    {
        if (ws.empty())
            return {};

        int sizeNeeded = WideCharToMultiByte(
            CP_UTF8,
            0,
            ws.data(),
            (int)ws.size(),
            nullptr,
            0,
            nullptr,
            nullptr
        );

        std::string result(sizeNeeded, 0);

        WideCharToMultiByte(
            CP_UTF8,
            0,
            ws.data(),
            (int)ws.size(),
            result.data(),
            sizeNeeded,
            nullptr,
            nullptr
        );

        return result;
    }

    static bool CreateDirectoryIfNotExists(const std::wstring& dirPath)
    {
        namespace fs = std::filesystem;

        // 1. 已存在直接返回
        if (fs::exists(dirPath))
            return true;

        // 2. 创建目录（递归）
        std::error_code ec;
        bool ok = fs::create_directories(dirPath, ec);

        return ok && !ec;
    }

}

namespace keyutils
{
    inline int ModifierVkToIndex(const USHORT& vk)
    {
        switch (vk)
        {
            // Control -> index 0 (不区分左右)
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_CONTROL:    return 0;

            // Shift -> index 2 (不区分左右)
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_SHIFT:      return 2;

            // Alt (Menu) -> index 4 (不区分左右)
        case VK_LMENU:
        case VK_RMENU:
        case VK_MENU:       return 1;
        
            // Win -> index 6 (不区分左右)
        case VK_LWIN:
        case VK_RWIN:       return 3;

        default: return -1;
        }
    }
   
    inline USHORT IndexToModifierVk(int index)
    {
        switch (index)
        {
        case 0: return VK_CONTROL;
        case 2: return VK_SHIFT;
        case 1: return VK_MENU;
        case 3: return VK_LWIN;
        default: return -1;
        }
    }
    inline std::vector<USHORT> ParseModifierMask(uint8_t mask)
    {
        std::vector<USHORT> mods;
        for (int i = 0; i < 8; i ++)
        {
            if (mask & (1 << i))
            {
                USHORT vk = IndexToModifierVk(i);
                mods.push_back(vk);
            }
        }
        return mods;
    }

    inline uint8_t BuildModifierMask(const std::vector<USHORT>& mods)
    {
        uint8_t mask = 0;
        for (USHORT vk : mods)
        {
            int idx = ModifierVkToIndex(vk);
            if (idx >= 0)
                mask |= (1 << idx);
        }
        return mask;
    }
}
