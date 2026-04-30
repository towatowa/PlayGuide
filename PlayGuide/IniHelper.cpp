#include "IniHelper.h"
#include <filesystem>


IniHelper::IniHelper(std::wstring const& path)
    : m_path(path)
{
    namespace fs = std::filesystem;

    if (fs::exists(m_path))
        return;

    // 1. 创建空文件
    HANDLE hFile = CreateFileW(
        m_path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    // 2. 写默认配置
    WritePrivateProfileStringW(L"Window", L"X", L"100", m_path.c_str());
    WritePrivateProfileStringW(L"Window", L"Y", L"100", m_path.c_str());
    WritePrivateProfileStringW(L"Window", L"Width", L"1280", m_path.c_str());
    WritePrivateProfileStringW(L"Window", L"Height", L"720", m_path.c_str());
    WritePrivateProfileStringW(L"Window", L"Maximized", L"0", m_path.c_str());
    WritePrivateProfileStringW(L"Window", L"alpha", L"255", m_path.c_str());

    WritePrivateProfileStringW(L"Hotkey", L"Play", L"192", m_path.c_str());
    WritePrivateProfileStringW(L"Hotkey", L"SeekAdd", L"54", m_path.c_str());
    WritePrivateProfileStringW(L"Hotkey", L"SeekDec", L"53", m_path.c_str());
    WritePrivateProfileStringW(L"Hotkey", L"Show", L"57", m_path.c_str());
    WritePrivateProfileStringW(L"Hotkey", L"OpacityAdd", L"56", m_path.c_str());
    WritePrivateProfileStringW(L"Hotkey", L"OpacityDec", L"55", m_path.c_str());

    WritePrivateProfileStringW(L"Web", L"Url",
        L"https://www.bilibili.com/",
        m_path.c_str());
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