#pragma once
#include <string>
#include <Windows.h>
#include "Appdata.h"

class IniHelper
{
public:
    IniHelper(std::wstring const& path);

    void WriteString(std::wstring const& section,
        std::wstring const& key,
        std::wstring const& value);

    std::wstring ReadString(std::wstring const& section,
        std::wstring const& key,
        std::wstring const& defaultValue = L"");

    void WriteInt(std::wstring const& section,
        std::wstring const& key,
        int value);

    int ReadInt(std::wstring const& section,
        std::wstring const& key,
        int defaultValue = 0);

private:
    std::wstring m_path;
};
