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

bool Win32Helper::IsRunningAsAdmin()
{
    BOOL isAdmin = FALSE;

    SID_IDENTIFIER_AUTHORITY NtAuthority =
        SECURITY_NT_AUTHORITY;

    PSID adminGroup = nullptr;

    if (AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminGroup))
    {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin;
}

void Win32Helper::RestartAsAdmin()
{
    wchar_t exePath[MAX_PATH]{};

    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";
    sei.lpFile = exePath;
    sei.lpParameters = L"--elevated";
    sei.nShow = SW_SHOWNORMAL;
    
    if (ShellExecuteExW(&sei))
    {
        exit(0);
    }
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


bool Win32Helper::IsIntelHybridCPU()
{
    int cpuInfo[4]{};
    __cpuid(cpuInfo, 0);

    char vendor[13]{};
    memcpy(vendor + 0, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[3], 4);
    memcpy(vendor + 8, &cpuInfo[2], 4);
    vendor[12] = 0;

    if (strcmp(vendor, "GenuineIntel") != 0)
        return false;

    ULONG len = 0;
    GetSystemCpuSetInformation(nullptr, 0, &len, GetCurrentProcess(), 0);

    if (len == 0)
        return false;

    std::vector<BYTE> buffer(len);
    auto info = reinterpret_cast<PSYSTEM_CPU_SET_INFORMATION>(buffer.data());

    if (!GetSystemCpuSetInformation(info, len, &len, GetCurrentProcess(), 0))
        return false;

    // 判断是否存在 E-core（EfficiencyClass > 0）
    BYTE* ptr = buffer.data();
    BYTE* end = buffer.data() + len;

    while (ptr < end)
    {
        auto cpu = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(ptr);

        if (cpu->Type == CpuSetInformation)
        {
            if (cpu->CpuSet.EfficiencyClass > 0)
                return true; // 存在E核 => hybrid
        }

        ptr += cpu->Size;
    }

    return false;
}

bool Win32Helper::SetThreadToEfficientCores()
{
    DWORD bufferLength = 0;
    // 第一次获取需要的缓冲区大小
    GetLogicalProcessorInformationEx(RelationAll, nullptr, &bufferLength);
    if (bufferLength == 0) {
        return false;
    }

    std::vector<BYTE> buffer(bufferLength);
    // 获取 CPU 信息
    BOOL success = GetLogicalProcessorInformationEx(
        RelationAll,
        (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data(),
        &bufferLength
    );

    if (!success) {
        return false;
    }

    // 用来保存所有 E 核的亲和掩码
    DWORD_PTR affinityMask = 0;

    // 遍历所有 CPU 信息项
    BYTE* ptr = buffer.data();
    while (ptr < buffer.data() + bufferLength)
    {
        auto* info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)ptr;


        // 只有处理器核心关系有效率等级
        if (info->Relationship == RelationProcessorCore)
        {
            BYTE efficiency = info->Processor.EfficiencyClass;
            WORD group = info->Processor.GroupMask[0].Group;
            DWORD_PTR mask = info->Processor.GroupMask[0].Mask;

            // ==========================================
            // E 核：efficiency >= 1
            // ==========================================
            if (efficiency <= 0)
            {
                affinityMask |= mask; // 把 E 核加入亲和掩码

                // 输出日志：找到 E 核
                wchar_t log[256] = { 0 };
                swprintf_s(log, L"[E 核] Group=%d, Mask=0x%08llX\n", group, mask);
                LOG_INFO << std::wstring(log);

                // 打印每个逻辑 CPU 号
                for (DWORD i = 0; i < 64; ++i) {
                    if (mask & (1ULL << i)) {
                        wchar_t cpuLog[64];
                        swprintf_s(cpuLog, L"→ 逻辑CPU %d\n", i);
                        LOG_INFO << std::wstring(cpuLog);
                    }
                }
            }
        }

        // 下一个结构体
        ptr += info->Size;
    }

    // 如果没有 E 核，直接返回成功（不绑核）
    if (affinityMask == 0) {
        LOG_INFO << (L"未找到 E 核，不设置亲和性\n");
        return true;
    }

    // 设置线程亲和到所有 E 核
    BOOL result = ::SetThreadAffinityMask(GetCurrentThread(), affinityMask);
    if (result) {
        LOG_INFO<<(L"✅ 线程已成功绑定到 E 核\n");
    }
    else {
        LOG_INFO << (L"❌ 绑定 E 核失败\n");
    }

    return result != FALSE;
}

void Win32Helper::ClearCpuAffinity()
{
    DWORD_PTR processMask = 0;
    DWORD_PTR systemMask = 0;

    // 1. 获取当前进程原本被允许使用的全部 CPU 掩码
    if (::GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask))
    {
        // 2. 将当前线程的硬亲和性还原为进程的完整掩码
        // 执行后，任务管理器中原本变灰的 P 核勾选框会重新全部勾上
        ::SetThreadAffinityMask(GetCurrentThread(), processMask);
    }
}

void Win32Helper::TestEfficientThread()
{
    // 只有产生持续计算，Thread Director 才会开始迁移核心
    for (int i = 0; i < 10; i++)
    {
        // 跑一小段死循环，产生大约 100ms 的纯计算负载
        auto start = std::chrono::steady_clock::now();
        volatile int dummy = 0;
        while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(100))
        {
            dummy++; // 制造不被编译器优化的计算
        }
        // 此时打印，看内核把线程调度到了哪里
        DWORD cpu = GetCurrentProcessorNumber();
        std::wstring out = L"Running on CPU:" + std::to_wstring(cpu) + L"\n";
        LOG_INFO << out;

        // 稍微歇一下再继续
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool Win32Helper::SetThreadToEfficientCoresV2()
{
    ULONG len = 0;
    GetSystemCpuSetInformation(nullptr, 0, &len, GetCurrentProcess(), 0);

    if (len == 0)
        return false;

    std::vector<BYTE> buffer(len);
    auto info = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(buffer.data());

    if (!GetSystemCpuSetInformation(info, len, &len, GetCurrentProcess(), 0))
        return false;

    std::vector<UINT32> ecores;

    BYTE* ptr = buffer.data();
    BYTE* end = buffer.data() + len;
    std::wstring out = L"E core numbers: ";
    while (ptr < end)
    {
        auto cpu = reinterpret_cast<SYSTEM_CPU_SET_INFORMATION*>(ptr);

        if (cpu->Type == CpuSetInformation)
        {
            if (cpu->CpuSet.EfficiencyClass <= 0) {
                ecores.push_back(cpu->CpuSet.Id);
                out += std::to_wstring(cpu->CpuSet.Id) + L",";
            }
        }

        ptr += cpu->Size;
    }

    if (ecores.empty())
        return false;
    out += L"\n";
    LOG_INFO << out;

    return SetThreadSelectedCpuSets(
        GetCurrentThread(),
        (ULONG*)ecores.data(),
        (ULONG)ecores.size()
    );
}

void Win32Helper::ClearCpuAffinityV2()
{
    SetThreadSelectedCpuSets(GetCurrentThread(), nullptr, 0);
}