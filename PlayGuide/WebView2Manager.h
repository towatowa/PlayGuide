#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.Foundation.h>
#include <filesystem>
#include "Win32Helper.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Microsoft::Web::WebView2::Core;

class WebView2Manager
{
public:
    static WebView2Manager& Instance()
    {
        static WebView2Manager inst;
        return inst;
    }

    winrt::Microsoft::Web::WebView2::Core::CoreWebView2Environment Environment()
    {
        return m_env;
    }

    winrt::Windows::Foundation::IAsyncAction Initialize()
    {
        if (m_initialized)
            co_return;

        auto userData = Win32Helper::GetLocalAppDataPath();

        std::filesystem::create_directories(userData);

        m_env =
            co_await CoreWebView2Environment::CreateWithOptionsAsync(
                L"",
                userData.c_str(),
                nullptr
            );

        m_initialized = true;
    }

private:
    bool m_initialized{ false };
    winrt::Microsoft::Web::WebView2::Core::CoreWebView2Environment m_env{ nullptr };
};
