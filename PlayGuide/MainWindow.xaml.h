#pragma once
#include "MainWindow.g.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <filesystem>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <CommCtrl.h>
#include "Event.h"
#include "Appdata.h"
#include "WebViewPage.xaml.h"

using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI::Dispatching;
using namespace winrt::Windows::Foundation;
using namespace Microsoft::Web::WebView2::Core;

namespace winrt::PlayGuide::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		HWND GetHWND(winrt::Microsoft::UI::Xaml::Window const& window);

		MainWindow(const hstring& url);

		void MainInitialize(HWND hwnd);
	   
		hstring Url() noexcept {
			return m_url;
		}
		void Url(hstring const& value) noexcept {
			m_url = value;
		}
		void SetHwnd(HWND hwnd) noexcept;
		void PlayPause() noexcept;
		void Seek(int sec) noexcept;
		void ShowHideWindow() noexcept;
		void HandleEvent(UINT msg) noexcept;
		void ApplyWindowState(const Appdata&state) noexcept;
		Appdata CaptureWindowData() noexcept;

		void CreateWebViewPage(hstring url) noexcept;
		void DeleteWebViewPage(int index) noexcept;
		void NavigatedTo(int index) noexcept;

		void SetTabCloseEvent(Event<int>& event);
		void SetNewUrlEnterEvent(Event<TabInfo>& event);
		void SetTabSeletedChangedEvent(Event<int>& event);
		Event<bool> controlWindowVisible;
		Event<bool> controlWindowHideEvent;
		Event<bool> controlWindowCloseEvent;
		//tabview event
		Event<int>::EventRevoker  tabCloseEvent;
		Event<TabInfo>::EventRevoker  newUrlEnterEvent;
		Event<int>::EventRevoker  tabSeletedChangedEvent;
		Event<TabInfo> pageCreatedStateEvent;
		Event<TabInfo>::EventRevoker webViewComplatedEventRevoker;
	private:
		hstring m_url{ L"https://www.bilibili.com/" };

		Event<SimpleEvent>::EventRevoker m_pipeClientHandleRevoker;
	  
		HWND m_hwnd{ nullptr };
		bool m_isCmdActived{ false };

		std::vector<IInspectable> m_webViewPages;
		bool m_alive{ true };
		int m_curIndex{ 0 };
	};
}

namespace winrt::PlayGuide::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
