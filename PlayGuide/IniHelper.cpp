#include "IniHelper.h"
#include <filesystem>


IniHelper::IniHelper(std::wstring const& path)
    : m_path(path)
{
   
}

void IniHelper::WriteString(std::wstring const& section,
    std::wstring const& key,
    std::wstring const& value)
{
    WritePrivateProfileStringW(
        section.c_str(),
        key.c_str(),
        value.c_str(),
        m_path.c_str()
    );
}

std::wstring IniHelper::ReadString(std::wstring const& section,
    std::wstring const& key,
    std::wstring const& defaultValue)
{
    wchar_t buffer[512]{};
    GetPrivateProfileStringW(
        section.c_str(),
        key.c_str(),
        defaultValue.c_str(),
        buffer,
        512,
        m_path.c_str()
    );
    return buffer;
}

void IniHelper::WriteInt(std::wstring const& section,
    std::wstring const& key,
    int value)
{
    WriteString(section, key, std::to_wstring(value));
}

int IniHelper::ReadInt(std::wstring const& section,
    std::wstring const& key,
    int defaultValue)
{
    return GetPrivateProfileIntW(
        section.c_str(),
        key.c_str(),
        defaultValue,
        m_path.c_str()
    );
}