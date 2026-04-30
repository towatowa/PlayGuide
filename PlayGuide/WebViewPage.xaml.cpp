#include "pch.h"
#include "WebViewPage.xaml.h"
#if __has_include("WebViewPage.g.cpp")
#include "WebViewPage.g.cpp"
#endif
#include "Logger.h"
#include <winrt/Windows.Storage.h>
#include "Win32Helper.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
Event<TabInfo> g_webViewComplatedEvent;

namespace winrt::PlayGuide::implementation
{
	decltype(WebViewPage::m_webView2Environment) WebViewPage::m_webView2Environment = nullptr;
	decltype(WebViewPage::m_localFolder)WebViewPage::m_localFolder{ L"" };

	WebViewPage::WebViewPage()
	{
	}
	WebViewPage::WebViewPage(hstring url)
	{
		m_url = url;
		auto weak_this = this->get_weak();
		//这里 webview2的初始化时机问题，由于Loaded是在加入 Visual Tree 才会 Loaded（就是设置为导航页时）
		//因此如果在Loaded初始化会有白屏，正确的逻辑时机选择在Loaded更合适, 后续可以做WebView 预加载池优化响应速度
		/*DispatcherQueue().TryEnqueue([weak_this]() {
			if (auto self = weak_this.get()) {
				self->InitializeWebView();
			}
			});
		*/
		this->Loaded([weak_this](auto&&, auto&&) {
			if (auto self = weak_this.get()) {
				self->InitializeWebView();
			}
			});
	}

	void WebViewPage::PlayPause() noexcept
	{
		webView.ExecuteScriptAsync(LR"(
        (() => {
            const v = document.querySelector('video');
            if (v) v.paused ? v.play() : v.pause();
        })();
    )");
	}

	void WebViewPage::Seek(int sec) noexcept
	{
		std::wstring js = LR"(
        (() => {
            const v = document.querySelector('video');
            if (v) v.currentTime += )" + std::to_wstring(sec) + LR"(;
        })();
    )";

		webView.ExecuteScriptAsync(js);
	}

	void WebViewPage::Close()
	{
		if (webView)
		{
			try
			{
				if (auto core = webView.CoreWebView2())
				{
					core.Stop();
				}

				webView.Close();
			}
			catch (...)
			{
			}

			webView = (nullptr);
		}
	}

	hstring WebViewPage::GetUrl() noexcept
	{
		return webView.CoreWebView2().Source();
	}

	hstring WebViewPage::WebTitle() noexcept
	{
		return webView.CoreWebView2().DocumentTitle();
	}

	winrt::Windows::Foundation::IAsyncAction WebViewPage::InitializeEnvironment()
	{
		// 👇 创建带参数的环境
		winrt::Microsoft::Web::WebView2::Core::CoreWebView2EnvironmentOptions options;
		options.AdditionalBrowserArguments(
			L"--disable-background-timer-throttling "
			L"--disable-backgrounding-occluded-windows "
			L"--disable-renderer-backgrounding "
			L"--autoplay-policy=no-user-gesture-required"
		);

		if (m_localFolder.empty()) {
			m_localFolder = Win32Helper::GetLocalAppDataPath(); //winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path();
			m_localFolder = m_localFolder / "PlayGuide_Data";
			if (!utils::CreateDirectoryIfNotExists(m_localFolder)) {
				LOG_ERROR << "CreateDirectoryIfNotExists failed.\n";
			}
		}

		if (!m_webView2Environment) {
			m_webView2Environment = co_await winrt::Microsoft::Web::WebView2::Core::CoreWebView2Environment::CreateWithOptionsAsync(
				L"",
				m_localFolder.c_str(),// userDataFolder（可自定义）
				options
			);
		}
	}

	winrt::Windows::Foundation::IAsyncAction WebViewPage::InitializeWebView()
	{
		if (webView) co_return; //页面已经存在不需要初始化
		try {
			CreateWebView();
			auto weak_this = get_weak();
			if (!m_webView2Environment) {
				co_await InitializeEnvironment();
			}
			co_await webView.EnsureCoreWebView2Async(m_webView2Environment);
			auto core = webView.CoreWebView2();

			if (!core) {
				LOG_ERROR << "webView.CoreWebView2 is return nullptr.\n";
				throw std::runtime_error("webView.CoreWebView2 is return nullptr.");
			}

			core.NewWindowRequested([weak_this](auto const& sender, auto const& args)
				{
					if (auto self = weak_this.get())
					{
						args.Handled(true);
						auto core = self->webView.CoreWebView2();
						if (core)
						{
							core.Navigate(args.Uri());
						}
					}
				});
			// 核心：页面加载完成事件
			core.NavigationCompleted(
				[weak_this](auto&& sender, auto&& args)
				{
					if (auto self = weak_this.get())
					{
						// ==========================================
						//         ✅✅✅ 页面加载完成✅✅✅
						// ==========================================
						bool isSuccess = args.IsSuccess();       // 是否加载成功
						hstring url = sender.Source();           // 最终 URL
						hstring title = sender.DocumentTitle();  // 网页标题

						LOG_INFO << L"页面加载完成：" << title.c_str() << L"\n";

						// 触发事件通知 MainWindow
						TabInfo info{ 0, title.c_str(), url.c_str() };
						g_webViewComplatedEvent.Invoke(info);
					}
				});

			webView.Source(winrt::Windows::Foundation::Uri(m_url));
		}
		catch (std::exception e)
		{
			std::string str = "InitWebView Error! " + std::string(e.what()) + "\n";
			LOG_ERROR << str;
		}
	}

	void WebViewPage::CreateWebView()
	{
		webView = winrt::Microsoft::UI::Xaml::Controls::WebView2();

		RootGrid().Children().Append(webView);
	}
}