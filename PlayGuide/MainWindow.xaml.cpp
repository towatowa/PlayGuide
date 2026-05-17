#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
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
		
		/*
		this->m_pipeClientHandleRevoker =
			PipeClient::Get().handler(auto_revoke, [weak_this](SimpleEvent msg)
				{
					if (auto self = weak_this.get())
					{
						std::wstring title = Win32Helper::GetWindowTitle(msg.hwnd);
						if (title != L"PlayGuide")
							self->HandleEvent(msg.vk);
					}
				});
		*/

		this->Closed([weak_this](auto const&, auto const&args)
			{
				if (auto self = weak_this.get()) {
					auto pages = self->m_webViewPages;
					self->m_webViewPages.clear();

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

		webViewComplatedEventRevoker = g_webViewComplatedEvent(auto_revoke, [weak_this](TabInfo info) 
		{
			if (auto self = weak_this.get())
			{
				self->pageCreatedStateEvent.Invoke(info);
			}
		});
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
				self->RootFrame().Content(make<SettingsPage>());
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
		auto page = this->m_webViewPages[m_curIndex].try_as<PlayGuide::WebViewPage>();
		page.PlayPause();
	}

	void MainWindow::Seek(int sec) noexcept
	{
		auto page = this->m_webViewPages[m_curIndex].try_as<PlayGuide::WebViewPage>();
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
			DispatcherQueue().TryEnqueue([weak_this]() {
				if (auto self = weak_this.get()) {
					self->ShowHideWindow();
				}
				});
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

		if (outOfBounds)
		{
			// 居中
			x = work.left + (work.right - work.left - state.width) / 2;
			y = work.top + (work.bottom - work.top - state.height) / 2;
		}
		else
		{
			x = rc.left;
			y = rc.top;
		}

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
			ShowWindow(m_hwnd, SW_HIDE);
			break;
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
        //state.url = m_webViewPages[m_curIndex].try_as<PlayGuide::WebViewPage>().GetUrl();

		//热键在后台service进程里写
		AppDataService::Get().SaveMainData(state);
		
	}

	void MainWindow::CreateWebViewPage(hstring url, int idx) noexcept
	{
		auto weak_this = this->get_weak();
		DispatcherQueue().TryEnqueue([weak_this, url, idx]() {
			auto self = weak_this.get();
			if (!self) return;
			self->m_webViewPages[idx] = make<PlayGuide::implementation::WebViewPage>(url, idx);
			//立即切换页面
			self->RootFrame().Content(self->m_webViewPages[idx]);
			self->m_curIndex = idx;
			
			LOG_INFO << "Created  a page " << std::wstring(url.c_str()) << "\n";
	     });
	}

	void MainWindow::DeleteWebViewPage(int index) noexcept
	{
		if (index == 0) return;

		auto weak_this = this->get_weak();
		DispatcherQueue().TryEnqueue([weak_this, index]() {
			auto self = weak_this.get();
			if (!self) return;
			if (index < 0)
				return;

			if (index == self->m_curIndex)
				self->RootFrame().Content(nullptr);
			self->m_webViewPages[index].try_as<PlayGuide::WebViewPage>().Close();
			self->m_webViewPages.erase(index);
			LOG_INFO << "Deleted page " << index << "\n";
			});
	}

	void MainWindow::NavigatedTo(int index) noexcept
	{
		if (index == m_curIndex)
			return;
		auto weak_this = this->get_weak();
		DispatcherQueue().TryEnqueue([weak_this, index]() {
			auto self = weak_this.get();
			if (!self) return;
			self->RootFrame().Content(self->m_webViewPages[index]);
			self->m_curIndex = index;
			
			LOG_INFO << "Navigated to page " << index << "\n";
			});
	}

	void MainWindow::SetTabCloseEvent(Event<int>& event)
	{
		tabCloseEvent = event(auto_revoke, [this](int idx) {
			DeleteWebViewPage(idx);
		});
	}

	void MainWindow::SetNewUrlEnterEvent(Event<TabInfo>& event)
	{
		newUrlEnterEvent = event(auto_revoke, [this](TabInfo info) {
			m_curIndex = info.idx;
			CreateWebViewPage(info.url.c_str(), info.idx);
			});
	}

	void MainWindow::SetTabSeletedChangedEvent(Event<int>& event)
	{
		tabSeletedChangedEvent = event(auto_revoke, [this](int idx) 
		{
			NavigatedTo(idx);
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
}

