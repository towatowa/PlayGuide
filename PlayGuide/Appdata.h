#pragma once
#include "Windows.h"
#include <string>

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
    int idx{ 0 };
    std::wstring title{ L"" };
    std::wstring url{ L"" };
};

struct Appdata
{
    int x{ 100 };
    int y{ 100 };
    int width{ 1280 };
    int height{ 720 };
    bool maximized{ false };
    int alpha{ 255 };//透明度
    USHORT play{ 192 };
    USHORT SeekAdd{ 54 };
    USHORT SeekDec{ 53 };
    USHORT OpacityAdd{ 56 };
    USHORT OpacityDec{ 55 };
    USHORT show{ 57 };
    std::wstring url{ L"https://www.bilibili.com/" };
};