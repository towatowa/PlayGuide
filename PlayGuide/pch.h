#pragma once
// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime

// 必须放在所有winrt头文件之前！
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

#include <Windows.h>
#include <Unknwn.h> // 用系统自带的 IUnknown，不要自己定义
#include <shlobj.h>
// 只定义一次，全局可见
#if !defined(IWindowNative_DEFINED)
#define IWindowNative_DEFINED

// 标准方式声明 COM 接口，不会被重定义
DECLARE_INTERFACE_IID_(IWindowNative, IUnknown, "EECDBF0E-BAE9-4CB6-A68E-9598E1CB57BB")
{
    STDMETHOD(get_WindowHandle)(_Out_ HWND * hWnd) PURE;
};
#endif


#include <restrictederrorinfo.h>
#include <hstring.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <wil/cppwinrt_helpers.h>
#pragma comment(lib, "Comctl32.lib")

using namespace winrt;

//#include "winrt/PlayGuide.h"