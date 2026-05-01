#pragma once
#include "Windows.h"
#include <string>
#include <unordered_map>

constexpr UINT WM_PLAY = WM_USER + 1;
constexpr UINT WM_SEEK_ADD = WM_USER + 2;
constexpr UINT WM_SEEK_DEC = WM_USER + 3;
constexpr UINT WM_SHOW_HIDE_WINDOW = WM_USER + 4;//窗口显示或者隐藏
constexpr UINT WM_OPACITY_ADD = WM_USER + 5;//透明度+
constexpr UINT WM_OPACITY_DEC = WM_USER + 6;

struct SimpleEvent
{
    HWND hwnd;
    USHORT vk;
};

struct TabInfo
{
    uint32_t idx{ 0 };
    std::wstring title{ L"" };
    std::wstring url{ L"" };
};

struct Appdata
{
    int x{ 100 };
    int y{ 100 };
    int width{ 1280 };
    int height{ 720 };
    int alpha{ 255 };//透明度
    virtual ~Appdata() = default; // ⭐关键
};

struct MainWindowData : Appdata
{ 
    bool maximized{ false };
    std::wstring url;
};

struct ControlWindowData : Appdata
{

};

using HotKeyMap = std::unordered_map<std::wstring, USHORT>;

inline HotKeyMap g_defaultHotkeys{
       {L"Opacity_Add", 56},
       {L"Opacity_Dec", 55},
       {L"Play", 192},
       {L"Seek_Add", 54},
       {L"Seek_Dec", 53},
       {L"Show_Hide_Window", 57}
};

inline std::wstring g_defaultWebUrl{ L"https://www.bilibili.com/" };

