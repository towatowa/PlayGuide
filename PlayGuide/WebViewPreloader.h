#pragma once
#include <winrt/Microsoft.Web.WebView2.Core.h>

namespace PlayGuide {
    struct WebViewPreloader
    {
        static winrt::fire_and_forget Prewarm()
        {
            // ⚠️ 提前创建 WebView2 environment（关键）
            //co_await winrt::resume_background(); // 可选
            auto m_env = co_await winrt::Microsoft::Web::WebView2::Core::CoreWebView2Environment::CreateAsync();
        }
    };
}
