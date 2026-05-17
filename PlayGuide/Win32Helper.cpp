#include "pch.h"
#include "Win32Helper.h"

void Win32Helper::SetSize(HWND hwnd, int width, int height)
{
    if (!hwnd) return;

    SetWindowPos(hwnd, nullptr, 0, 0, width, height,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

RECT Win32Helper::GetWorkArea(HWND hwnd)
{
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(monitor, &mi);

    return mi.rcWork;
}

void Win32Helper::Center(HWND hwnd)
{
    if (!hwnd) return;

    RECT rc{};
    GetWindowRect(hwnd, &rc);

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    RECT work = GetWorkArea(hwnd);

    int x = work.left + (work.right - work.left - w) / 2;
    int y = work.top + (work.bottom - work.top - h) / 2;

    SetWindowPos(hwnd, nullptr, x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER);
}

void Win32Helper::SetTopMost(HWND hwnd, bool topMost=true)
{
    if (!hwnd) return;

    SetWindowPos(hwnd,
        topMost ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE);
}

void Win32Helper::EnableLayered(HWND hwnd)
{
    LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
}

void Win32Helper::SetOpacity(HWND hwnd, BYTE alpha)
{
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

BYTE Win32Helper::GetOpacity(HWND hwnd)
{
    BYTE alpha = 255;
    DWORD flags{};
    COLORREF key{};

    if (GetLayeredWindowAttributes(hwnd, &key, &alpha, &flags))
        return alpha;

    return 255;
}

void Win32Helper::ShowHide(HWND hwnd)
{
    if (!hwnd) return;

    ShowWindow(hwnd,
        IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
}

void Win32Helper::ApplyStyle(HWND hwnd, LONG removeStyle)
{
    if (!hwnd) return;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    style &= ~removeStyle; // remove flag

    SetWindowLong(hwnd, GWL_STYLE, style);

    // ⭐ 关键：刷新窗口非客户区
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void Win32Helper::HideMinimizeButton(HWND hwnd)
{
    ApplyStyle(hwnd, WS_MINIMIZEBOX);
}

void Win32Helper::HideMaximizeButton(HWND hwnd)
{
    ApplyStyle(hwnd, WS_MAXIMIZEBOX);
}

void Win32Helper::DisableResize(HWND hwnd)
{
    ApplyStyle(hwnd, WS_THICKFRAME);
}

int Win32Helper::GetRefreshRate(HWND hwnd)
{
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    DEVMODE dm{};
    dm.dmSize = sizeof(dm);

    if (EnumDisplaySettings(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm))
    {
        return dm.dmDisplayFrequency; // Hz
    }

    return 60; // fallback
}

void Win32Helper::RemoveFrame(HWND hwnd)
{
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME);

    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    ApplyFrameChange(hwnd);
}

void Win32Helper::RestoreFrame(HWND hwnd)
{
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style |= (WS_CAPTION | WS_THICKFRAME);

    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    ApplyFrameChange(hwnd);
}

void Win32Helper::ApplyFrameChange(HWND hwnd)
{
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
}

std::wstring Win32Helper::GetWindowTitle(HWND hwnd)
{
    int len = GetWindowTextLengthW(hwnd);
    if (len <= 0)
        return L"";

    std::wstring title(len, L'\0');
    GetWindowTextW(hwnd, title.data(), len + 1);

    return title;
}

/*
// 获取真实物理路径
std::wstring Win32Helper::GetLocalPath(const std::wstring &name) {
    wchar_t path[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);
    std::filesystem::path localPath = path;
    localPath /= name; // 必须手动加上应用名
    std::filesystem::create_directories(localPath);
    return localPath.wstring();
}
*/

void Win32Helper::SetCurrentDirToExePath()
{
    wchar_t path[MAX_PATH]{};

    // 获取 exe 完整路径
    GetModuleFileNameW(nullptr, path, MAX_PATH);

    std::wstring fullPath = path;

    // 去掉文件名，只保留目录
    auto pos = fullPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        std::wstring dir = fullPath.substr(0, pos);
        SetCurrentDirectoryW(dir.c_str());
    }
}

std::filesystem::path Win32Helper::GetExeDir()
{
    wchar_t path[MAX_PATH]{};

    // 获取 exe 完整路径
    GetModuleFileNameW(nullptr, path, MAX_PATH);

    std::wstring fullPath = path;

    // 去掉文件名，只保留目录
    auto pos = fullPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        std::wstring dir = fullPath.substr(0, pos);
        return std::filesystem::path(std::move(dir));
    }
    return L"";
}

std::wstring Win32Helper::GetLocalAppDataPath()
{
    PWSTR path = nullptr;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
    {
        std::wstring result(path);
        CoTaskMemFree(path);
        return result;
    }

    return L"";
}

void Win32Helper::SetAutoStart(bool enable)
{
    const wchar_t* regPath =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    HKEY hKey;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, regPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    {
        if (enable)
        {
            wchar_t path[MAX_PATH]{};
            GetModuleFileNameW(nullptr, path, MAX_PATH);

            RegSetValueExW(
                hKey,
                L"PlayGuide",
                0,
                REG_SZ,
                (BYTE*)path,
                (wcslen(path) + 1) * sizeof(wchar_t));
        }
        else
        {
            RegDeleteValueW(hKey, L"PlayGuide");
        }

        RegCloseKey(hKey);
    }
}
