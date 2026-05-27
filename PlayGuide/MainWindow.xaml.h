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
#include <winrt/Windows.Graphics.h>
#include <EventToken.h>


using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI::Dispatching;
using namespace winrt::Windows::Foundation;
using namespace Microsoft::Web::WebView2::Core;
using namespace winrt::Windows::Graphics;


namespace winrt::PlayGuide::implementation
{
	// 停靠方向
	enum class MainDockSide
	{
		None,
		LeftTop,
		LeftBottom,
		RightTop,
		RightBottom
	};
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

		Windows::Foundation::IAsyncAction CreateWebViewPage(hstring url, int idx) noexcept;
		void DeleteWebViewPage(int index) noexcept;
		Windows::Foundation::IAsyncAction NavigatedTo(const TabInfo& info) noexcept;

		void SetTabCloseEvent(Event<int>& event);
		void SetNewUrlRequestEvent(Event<const TabInfo&>& event);
		void SetTabSeletedChangedEvent(Event<const TabInfo&>& event);
		void SetPipeServiceHandleEvent(Event<UINT>& event);
		void SetSystemTrayClickEventRevoker(Event<>& event);

		void SetSystemTrayShowWindowRevoker(Event<>& event);

		void Grid_PointerPressed(IInspectable const& sender, Input::PointerRoutedEventArgs const& e);

		void Grid_PointerMoved(IInspectable const& sender, Input::PointerRoutedEventArgs const& e);

		void Grid_PointerReleased(IInspectable const& sender, Input::PointerRoutedEventArgs const& e);

        RectInt32 GetScreenWorkArea() noexcept;

		MainDockSide CheckDockSide(const RectInt32& windowBounds, const RectInt32& screen) noexcept;

		void InitDockTimer();

		void OnDragFinished();

		Event<bool> controlWindowVisible;
		Event<bool> controlWindowHideEvent;
		Event<bool> controlWindowCloseEvent;
		//tabview event
		Event<int>::EventRevoker  tabCloseEvent;
		Event<const TabInfo&>::EventRevoker  newUrlRequestEvent;
		Event<const TabInfo&>::EventRevoker  tabSeletedChangedEvent;
		Event<const TabInfo&> pageCreatedStateEvent;
		Event<const TabInfo&>::EventRevoker webViewComplatedEventRevoker;

	private:
		hstring m_url{ L"https://www.bilibili.com/" };

		Event<UINT>::EventRevoker m_pipeServiceHandleRevoker;
		Event<>::EventRevoker m_systemTrayClickEventRevoker;
		Event<>::EventRevoker m_systemTrayShowWindowRevoker;
		HWND m_hwnd{ nullptr };
		//std::vector<IInspectable> m_pages;
		std::unordered_map<uint32_t, IInspectable>m_pages;
		uint32_t m_curIndex{ 65535 };

		WindowState m_curWinState{ WindowState::Normal };

		bool m_isDragging{ false };
		POINT       m_dragStartCursor{};
		PointInt32  m_dragStartWindowPos{};
		int         m_refreshRate{ 60 };
		std::chrono::steady_clock::time_point m_lastMoveTime;
		RectInt32   m_screenCache{};

		winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer m_dragTimer{ nullptr };

		winrt::event_token m_windowChangedToken{};
	};
}

namespace winrt::PlayGuide::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
