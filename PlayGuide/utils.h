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