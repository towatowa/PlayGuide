#pragma once
#include <Windows.h>
//#include <shlobj.h>
#include <filesystem>
#include <shlobj.h>

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
private:
    static RECT GetWorkArea(HWND hwnd);
    static void ApplyStyle(HWND hwnd, LONG removeStyle); 
    static void ApplyFrameChange(HWND hwnd);
    
};
