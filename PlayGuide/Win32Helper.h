#pragma once
#include <Windows.h>
//#include <shlobj.h>
#include <filesystem>
#include <shlobj.h>
#include <shellapi.h>
#include <intrin.h>
#include <vector>
#include "Logger.h"
#pragma comment(lib, "Shell32.lib")





using PFN_GetSystemCpuSetInformation = DWORD(WINAPI*)(
    PSYSTEM_CPU_SET_INFORMATION,
    DWORD,
    PDWORD,
    HANDLE
    );

class Win32Helper
{
public:
    static void SetSize(HWND hwnd, int width, int height);
    static void Center(HWND hwnd);
    static void SetTopMost(HWND hwnd, bool topMost);

    static BYTE GetOpacity(HWND hwnd);
    static void EnableLayered(HWND hwnd);
    static void SetOpacity(HWND hwnd, BYTE alpha);

    static void ShowHide(HWND hwnd);

    static void HideMinimizeButton(HWND hwnd);
    static void HideMaximizeButton(HWND hwnd);
    static void DisableResize(HWND hwnd);
    static int  GetRefreshRate(HWND hwnd);
    static void RemoveFrame(HWND hwnd);
    static void RestoreFrame(HWND hwnd);
    static std::wstring GetWindowTitle(HWND hwnd);
    static void SetCurrentDirToExePath();
    static std::filesystem::path GetExeDir();
    static std::wstring GetLocalAppDataPath();
    static bool IsRunningAsAdmin();
    static void RestartAsAdmin();
    static bool IsIntelHybridCPU();
    static bool SetThreadToEfficientCores();
    static void ClearCpuAffinity();
    static void TestEfficientThread();
    static bool SetThreadToEfficientCoresV2();
    static void ClearCpuAffinityV2();
    static void SetAutoStart(bool enable);
private:
    static RECT GetWorkArea(HWND hwnd);
    static void ApplyStyle(HWND hwnd, LONG removeStyle); 
    static void ApplyFrameChange(HWND hwnd);
    
};
