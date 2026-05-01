#pragma once

#include "WebViewPage.g.h"
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.Foundation.h>
#include "Appdata.h"
#include "Event.h"
#include <filesystem>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;

// 只声明，不创建实体
extern Event<TabInfo> g_webViewComplatedEvent;

namespace winrt::PlayGuide::implementation
{
    struct WebViewPage : WebViewPageT<WebViewPage>
    {
        WebViewPage();
        WebViewPage(hstring url, int idx);

        hstring Url() noexcept {
            return m_url;
        }
        void Url(hstring const& value) noexcept {
            m_url = value;
        }

        void PlayPause() noexcept;
        void Seek(int sec) noexcept;
        void Close();
        hstring GetUrl() noexcept;
        hstring WebTitle() noexcept;
        winrt::Windows::Foundation::IAsyncAction InitializeEnvironment();
        winrt::Windows::Foundation::IAsyncAction InitializeWebView();

        void CreateWebView();

    private:
        hstring m_url{ L"" };
        HWND m_hwnd{ nullptr };
        //winrt::com_ptr<Microsoft::Web::WebView2::Core::CoreWebView2Environment> m_webView2Environment;
        static winrt::Microsoft::Web::WebView2::Core::CoreWebView2Environment m_webView2Environment;
        static std::filesystem::path m_localFolder;
        winrt::Microsoft::UI::Xaml::Controls::WebView2 webView{ nullptr };
        int id{ 0 };
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct WebViewPage : WebViewPageT<WebViewPage, implementation::WebViewPage>
    {
    };
}
