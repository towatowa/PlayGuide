#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <winrt/Windows.System.h>
#include "Logger.h"
//#include "PipeClient.h"
//#include <shobjidl.h>
#include <winrt/Microsoft.UI.Interop.h>
#include "AppDataService.h"
#include "Win32Helper.h"
#include "SettingsPage.xaml.h"
#include "PipeService.h"
#include "TrayIconService.h"
#include "Global.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::PlayGuide::implementation
{
	MainWindow::MainWindow(const hstring& url)
	{	
		if (!url.empty())
		{
			m_url = url;
		}
		auto weak_this = get_weak();

		this->Closed([weak_this](auto const&, auto const&args)
			{
				if (auto self = weak_this.get()) {
					auto pages = self->m_pages;
					self->m_pages.clear();

					for (auto& wv : pages)
					{
						if (auto page = wv.second.try_as<PlayGuide::WebViewPage>())
						{
							page.Close();
						}
					}
					self->RootFrame().Content(nullptr);
					LOG_INFO << (L"Window state saved successfully.\n");
				}
			});
		
		
		this->Activated([weak_this](auto&&sender, auto&&args) {
			if (auto self = weak_this.get())
			{
				switch (args.WindowActivationState())
				{
				case WindowActivationState::CodeActivated:
				//case WindowActivationState::PointerActivated:
					LOG_DEBUG << L"主窗口激活\n";
					self->controlWindowVisible.Invoke(true);
					break;
				case WindowActivationState::Deactivated:
					LOG_DEBUG << L"主窗口失活\n";
					self->controlWindowVisible.Invoke(false);
					break;
				}
			}
			});

		InitDockTimer();
		m_windowChangedToken =
			AppWindow().Changed(
				[weak_this](auto const& sender, auto const& args)
				{
					auto self = weak_this.get();
					if (!self)
						return;

					if (args.DidPositionChange())
					{
						if (!self->m_dragTimer)
							return;

						self->m_dragTimer.Stop();

						self->m_dragTimer.Start();
					}
				});
		m_screenCache = GetScreenWorkArea();
	}


	void MainWindow::MainInitialize(HWND hwnd)
	{
		m_hwnd = hwnd;
		DispatcherQueue().TryEnqueue([weak_this = get_weak(), hwnd]()
			{
				auto self = weak_this.get();
				if (self == nullptr)
					return;
				self->AppWindow().SetIcon(L"Assets\\AppIcon.ico");
				self->ExtendsContentIntoTitleBar(true);
				//设置标题栏拖动区域
				self->AppWindow().TitleBar().SetDragRectangles({ winrt::Windows::Graphics::RectInt32{0, 0, 10000, 40} });
				if (auto presenter = self->AppWindow().Presenter().as<OverlappedPresenter>())
					presenter.IsAlwaysOnTop(true);
				MainWindowData state = AppDataService::Get().LoadMainData();

				self->ApplyWindowState(state);
				//self->CreateWebViewPage(state.url.c_str(), 0);
				self->m_pages[0] = make<SettingsPage>();//0 idx 为设置页
				//self->RootFrame().Content(make<SettingsPage>());
				PipeService::Get().SendFilterRule(hwnd);
				int cx = GetSystemMetrics(SM_CXSMICON);
				int cy = GetSystemMetrics(SM_CYSMICON);
#include "resource.h"
				HICON hIcon = reinterpret_cast<HICON>(
					::LoadImageW(
						::GetModuleHandleW(nullptr),
						MAKEINTRESOURCEW(IDI_ICON1),
						IMAGE_ICON,
						cx,
						cy,
						LR_DEFAULTCOLOR));

				TrayIconService::Get().Initialize(hwnd, hIcon, L"PlayGuide");
				if(AppDataService::Get().SystemTray())
				    TrayIconService::Get().Show();

				//重载appwindow().closing事件以实现窗口行为控制
				self->AppWindow().Closing([self](auto&&, auto&& args) {
					if (AppDataService::Get().SystemTray()) {
						args.Cancel(true);
						self->AppWindow().Hide();
						self->m_curWinState = WindowState::SystemTray;
						return;
					}
					else {
						g_processExitEvent.Invoke();
						return;
					}
				 });
			});
	}

	HWND MainWindow::GetHWND(winrt::Microsoft::UI::Xaml::Window const& window)
	{
		auto windowNative = window.as<::IWindowNative>();
		HWND hwnd = nullptr;
		windowNative->get_WindowHandle(&hwnd);
		return hwnd;
	}

	void MainWindow::SetHwnd(HWND hwnd) noexcept
	{
		m_hwnd = hwnd;
	}

	void MainWindow::PlayPause() noexcept
	{
		auto page = this->m_pages[m_curIndex].try_as<PlayGuide::WebViewPage>();
		if(page)
page.PlayPause();
	}

	void MainWindow::Seek(int sec) noexcept
	{
		auto page = this->m_pages[m_curIndex].try_as<PlayGuide::WebViewPage>();
		if(page)
            page.Seek(sec);
		//webView().ExecuteScriptAsync(js);
	}

	void MainWindow::ShowHideWindow() noexcept
	{
		if (!AppWindow())
			return;
		if (AppWindow().IsVisible())
		{
			AppWindow().Hide();
			//主窗口命令隐藏,则隐藏控制窗口
			controlWindowVisible.Invoke(false);
			m_curWinState = WindowState::Hidden;
		}
		else
		{
			AppWindow().Show();
			//controlWindowVisible.Invoke(true);//主窗口命令显示时不显示
			m_curWinState = WindowState::Normal;
		}
	}
	void MainWindow::MaximizeWindow() noexcept
	{
		this->AppWindow().SetPresenter(AppWindowPresenterKind::FullScreen);
		m_curWinState = WindowState::Maximized;
	}
	void MainWindow::ToggleMaximize() noexcept
	{
		auto presenter = this->AppWindow().Presenter().as<OverlappedPresenter>();

		if (presenter.State() == OverlappedPresenterState::Maximized)
		{
			presenter.Restore();
			m_curWinState = WindowState::Normal;
		}
		else
		{
			presenter.Maximize();
			m_curWinState = WindowState::Maximized;
		}
	}
	void MainWindow::HandleEvent(UINT msg) noexcept
	{
		auto weak_this = this->get_weak();
		switch (msg)
		{
		case WM_EnableHotkeys:
			break;
		case WM_PlayPause:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			DispatcherQueue().TryEnqueue([weak_this]() {
				if (auto self = weak_this.get()) {
					self->PlayPause();
				}
				});
			break;
		}
		case WM_SkipForward:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			DispatcherQueue().TryEnqueue([weak_this]() {
				if (auto self = weak_this.get())
					self->Seek(5);
				});
			break;
		}
		case WM_SkipBackward:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			DispatcherQueue().TryEnqueue([weak_this]() {
				if (auto self = weak_this.get())
					self->Seek(-5);
				});
			break;
		}
		case WM_ShowHideWindow:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			//DispatcherQueue().TryEnqueue([weak_this]() {
				//if (auto self = weak_this.get()) {
					//self->ShowHideWindow();
			Win32Helper::ShowHide(m_hwnd);
				//}
				//});
			break;
		}
		case WM_IncreaseOpacity:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			int alpha = (int)Win32Helper::GetOpacity(m_hwnd) - 10;
			if (alpha < 45) alpha = 45;
			Win32Helper::SetOpacity(m_hwnd, alpha);
			break;
		}
		case WM_DecreaseOpacity:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			int alpha = (int)Win32Helper::GetOpacity(m_hwnd) + 10;
			if(alpha >= 255) alpha = 255;
			Win32Helper::SetOpacity(m_hwnd, alpha);
			break;
		}
		case WM_MaximizeWindow:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			DispatcherQueue().TryEnqueue([weak_this]() {
				if (auto self = weak_this.get()) {
					self->ToggleMaximize();
				}
			});
			break;
		}
		default:
			break;
		}
	}

	void MainWindow::ApplyWindowState(const MainWindowData& state) noexcept
	{
		if (!m_hwnd)
			return;

		// ---- 获取当前显示器工作区（Win32）----
		HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi{};
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);

		RECT work = mi.rcWork;

		// ---- 目标窗口区域 ----
		RECT rc{
			state.x,
			state.y,
			state.x + state.width,
			state.y + state.height
		};

		// ---- 检查是否在屏幕内 ----
		bool outOfBounds =
			rc.left   < work.left ||
			rc.top    < work.top ||
			rc.right  > work.right ||
			rc.bottom > work.bottom;

		int x, y;

		x = rc.left;
		y = rc.top;
		

		// ---- 一次性设置位置 + 大小 ----
		SetWindowPos(
			m_hwnd,
			nullptr,
			x,
			y,
			state.width,
			state.height,
			SWP_NOZORDER | SWP_NOACTIVATE
		);
		m_curWinState = state.windowState;
		switch (state.windowState)
		{
		case WindowState::Maximized:
			ShowWindow(m_hwnd, SW_MAXIMIZE);
			break;

		case WindowState::Minimized:
			ShowWindow(m_hwnd, SW_MINIMIZE);
			break;
		case WindowState::SystemTray:
			//ShowWindow(m_hwnd, SW_HIDE);
			//break;
		case WindowState::Hidden:
		case WindowState::Normal:
		default:
			ShowWindow(m_hwnd, SW_RESTORE);
			break;
		}

		// ---- 透明度（Layered）----
		LONG ex = GetWindowLong(m_hwnd, GWL_EXSTYLE);
		if (!(ex & WS_EX_LAYERED))
		{
			SetWindowLong(m_hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
		}

		SetLayeredWindowAttributes(
			m_hwnd,
			0,
			static_cast<BYTE>(state.alpha),
			LWA_ALPHA
		);
	}

	void MainWindow::SaveWindowStateData() noexcept
	{
		if (!m_hwnd)
		{
			LOG_INFO << "SaveWindowStateData read m_hwnd is nullptr \n";
			return;
		}
		
		MainWindowData state;
		RECT rc{};
		GetWindowRect(m_hwnd, &rc);

		state.x = rc.left;
		state.y = rc.top;
		state.width = rc.right - rc.left;
		state.height = rc.bottom - rc.top;

		//获取窗的最大最小化状态
		state.windowState = m_curWinState;
		//窗口的透明度
		state.alpha = Win32Helper::GetOpacity(m_hwnd);

		//当前打开的url
        //state.url = m_pages[m_curIndex].try_as<PlayGuide::WebViewPage>().GetUrl();

		//热键在后台service进程里写
		AppDataService::Get().SaveMainData(state);
		m_dragTimer.Stop();
	}

	Windows::Foundation::IAsyncAction MainWindow::CreateWebViewPage(hstring url, int idx) noexcept
	{
		auto weak_this = get_weak();

		DispatcherQueue().TryEnqueue(
			[weak_this, url, idx]()
			{
				auto self = weak_this.get();
				if (!self)
					return;

				if (self->m_pages.contains(idx))
					return;

				self->m_pages[idx] =
					make<PlayGuide::implementation::WebViewPage>(
						url,
						idx);

				LOG_INFO << "Created page "
					<< idx
					<< "\n";
			});

		co_return;
	}

	void MainWindow::DeleteWebViewPage(int index) noexcept
	{
		auto weak_this = this->get_weak();
		DispatcherQueue().TryEnqueue([weak_this, index]() {
			auto self = weak_this.get();
			if (!self) return;
			if (index < 0)
				return;
			auto it = self->m_pages.find(index);
			if (it == self->m_pages.end())
				return;
			if (index == self->m_curIndex)
				self->RootFrame().Content(nullptr);
			if (index > 1) {
				if (auto page = self->m_pages[index].try_as<PlayGuide::WebViewPage>()) {
					page.Close();
					self->m_pages.erase(index);
				}
			}
		   
			LOG_INFO << "Deleted page " << index << "\n";
			});
	}

	Windows::Foundation::IAsyncAction MainWindow::NavigatedTo(const TabInfo& info) noexcept
	{
		if (m_curIndex == info.idx)
			co_return;

		if (!m_pages.contains(info.idx))
		{
			m_pages[info.idx] =
				make<PlayGuide::implementation::WebViewPage>(
					info.url.c_str(),
					info.idx);

			LOG_INFO << "Created page "
				<< info.idx
				<< "\n";
		}

		RootFrame().Content(m_pages[info.idx]);

		m_curIndex = info.idx;
	}

	void MainWindow::SetTabCloseEvent(Event<int>& event)
	{
		tabCloseEvent = event(auto_revoke, [this](int idx) {
			DeleteWebViewPage(idx);
		});
	}

	void MainWindow::SetNewUrlRequestEvent(Event<const TabInfo&>& event)
	{
		newUrlRequestEvent = event(auto_revoke, [this](const TabInfo &info) {
			m_curIndex = info.idx;
			CreateWebViewPage(info.url.c_str(), info.idx);
			});
	}

	void MainWindow::SetTabSeletedChangedEvent(Event<const TabInfo&>& event)
	{
		tabSeletedChangedEvent = event(auto_revoke, [this](const TabInfo& info)
		{
			NavigatedTo(info);
		});
	}

	void MainWindow::SetPipeServiceHandleEvent(Event<UINT>& event)
	{
		m_pipeServiceHandleRevoker = event(auto_revoke, [this](UINT msg) {
			this->HandleEvent(msg);
			});
	}

	void MainWindow::SetSystemTrayClickEventRevoker(Event<>& event)
	{
		m_systemTrayClickEventRevoker = event(auto_revoke, [this]() {
			AppWindow().Show();
			});
	}

	void MainWindow::SetSystemTrayShowWindowRevoker(Event<>& event)
	{
		m_systemTrayShowWindowRevoker = event(auto_revoke, [this]() {
			AppWindow().Show();
			});
	}

	RectInt32 MainWindow::GetScreenWorkArea() noexcept {
		DisplayArea displayArea =
			DisplayArea::GetFromPoint(
				Windows::Graphics::PointInt32{ 0, 0 },
				DisplayAreaFallback::Nearest
			);
		auto workArea = displayArea.WorkArea();
		return workArea;
	};

	MainDockSide MainWindow::CheckDockSide(const RectInt32& windowBounds, const RectInt32& screen) noexcept
	{
		constexpr int threshold = 15;

		bool isLeft =
			windowBounds.X <= threshold;

		bool isRight =
			screen.Width - (windowBounds.X + windowBounds.Width)
			<= threshold;

		bool isTop =
			windowBounds.Y <= (m_screenCache.Height / 2);

		bool isBottom =
			windowBounds.Y > (m_screenCache.Height / 2);

		if (isLeft)
		{
			if (isTop)
				return MainDockSide::LeftTop;

			if (isBottom)
				return MainDockSide::LeftBottom;
		}

		if (isRight)
		{
			if (isTop)
				return MainDockSide::RightTop;

			if (isBottom)
				return MainDockSide::RightBottom;
		}

		return MainDockSide::None;
	}

	void MainWindow::InitDockTimer()
	{
		auto weak_this = get_weak();

		m_dragTimer =
			DispatcherQueue().CreateTimer();

		m_dragTimer.Interval(
			std::chrono::milliseconds(500));

		m_dragTimer.IsRepeating(false);

		m_dragTimer.Tick(
			[weak_this](auto const&, auto const&)
			{
				auto self = weak_this.get();
				if (!self)
					return;

				self->OnDragFinished();
			});
	}

	void MainWindow::OnDragFinished()
	{
		if (!AppDataService::Get().GetEnableWindowSnapping())
			return;
		auto pos = AppWindow().Position();
		auto size = AppWindow().Size();

		RectInt32 rect{
			pos.X,
			pos.Y,
			size.Width,
			size.Height
		};

		auto side =
			CheckDockSide(rect, m_screenCache);

		switch (side)
		{
		case MainDockSide::LeftTop:

			AppWindow().Move({
				-10,
				-10
				});

			break;

		case MainDockSide::LeftBottom:

			AppWindow().Move({
				-10,
				m_screenCache.Height - size.Height + 10
				});

			break;

		case MainDockSide::RightTop:

			AppWindow().Move({
				m_screenCache.Width - size.Width+10,
				-10
				});

			break;

		case MainDockSide::RightBottom:

			AppWindow().Move({
				m_screenCache.Width - size.Width + 10,
				m_screenCache.Height - size.Height + 10
				});

			break;

		default:
			break;
		}
	}
}

