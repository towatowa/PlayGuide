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
#include <unordered_map>

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
		//keyboard event
		void PlayPause() noexcept;
		void Seek(int sec) noexcept;
		void ShowHideWindow() noexcept;
		void MaximizeWindow() noexcept;
		void ToggleMaximize() noexcept;
		void HandleEvent(UINT msg) noexcept;
		void ApplyWindowState(const MainWindowData&state) noexcept;
		void SaveWindowStateData() noexcept;

		void CreateWebViewPage(hstring url, int idx) noexcept;
		void DeleteWebViewPage(int index) noexcept;
		void NavigatedTo(int index) noexcept;

		void SetTabCloseEvent(Event<int>& event);
		void SetNewUrlEnterEvent(Event<TabInfo>& event);
		void SetTabSeletedChangedEvent(Event<int>& event);
		void SetPipeServiceHandleEvent(Event<UINT>& event);
		void SetSystemTrayClickEventRevoker(Event<>& event);

		void SetSystemTrayShowWindowRevoker(Event<>& event);

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

		Event<UINT>::EventRevoker m_pipeServiceHandleRevoker;
		Event<>::EventRevoker m_systemTrayClickEventRevoker;
		Event<>::EventRevoker m_systemTrayShowWindowRevoker;
		HWND m_hwnd{ nullptr };
		//std::vector<IInspectable> m_webViewPages;
		std::unordered_map<uint32_t, IInspectable>m_webViewPages;
		int m_curIndex{ 0 };

		WindowState m_curWinState{ WindowState::Normal };
	};
}

namespace winrt::PlayGuide::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
