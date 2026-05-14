#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>

namespace KeyMapping
{
    // 字符串 → VK
    USHORT KeyNameToVK(const std::string& name);

    // VK → 字符串
    std::string VKToKeyName(USHORT vk);

    // 获取完整映射（只读）
    const std::unordered_map<std::string, USHORT>& GetKeyNameToVKMap();
    const std::unordered_map<USHORT, std::string>& GetVKToKeyNameMap();
}
