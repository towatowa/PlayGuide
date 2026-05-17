#include "pch.h"
#include "ControlWindow.xaml.h"
#if __has_include("ControlWindow.g.cpp")
#include "ControlWindow.g.cpp"
#endif
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Input.h>   // ⭐关键
#include <winrt/Windows.Graphics.h>

#include "Win32Helper.h"
#include "Logger.h"
#include "PipeService.h"

using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Windowing;
using namespace Windows::Graphics;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Input;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::PlayGuide::implementation
{
	ControlWindow::ControlWindow()
	{
		auto weak_this = get_weak();
		auto dispatcherQueue = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
		m_hoverTimer = dispatcherQueue.CreateTimer();
		m_hoverTimer.Interval(std::chrono::milliseconds(2000)); // 延迟时间
		m_hoverTimer.IsRepeating(false); // 关键：只执行一次
		m_hoverTimer.Tick([weak_this](auto&&, auto&&)
			{
				auto self = weak_this.get();
				if (!self) return;
				auto winBounds = self->GetWindowRect();
				auto side = self->CheckDockSide(winBounds, self->m_screenCache);
				UINT dpi = GetDpiForWindow(self->m_hwnd);
				int expandWidth = ExpandWidth * dpi / 96.0f;
				int expandHeight = ExpandHeight * dpi / 96.0f;
				int dockSnapWidth = DockSnapWidth * dpi / 96.0f;
				int dockSnapHeight = DockSnapHeight * dpi / 96.0f;
				switch (side)
				{
				case DockSide::Left:
					self->AppWindow().MoveAndResize({ 0, winBounds.Y, dockSnapHeight, expandHeight });
					break;
				case DockSide::Right:
					self->AppWindow().MoveAndResize({ self->m_screenCache.Width - dockSnapWidth, winBounds.Y, dockSnapWidth, expandHeight });
					break;
				case DockSide::Top:
					self->AppWindow().MoveAndResize({ winBounds.X, 0, expandWidth, dockSnapHeight });
					break;
				default:
					break;
				}
			});

		this->Activated([weak_this](auto&&, auto&& args)
			{
				auto self = weak_this.get();
				if (!self) return;
				self->m_isActive = (args.WindowActivationState() != WindowActivationState::Deactivated);

				LOG_DEBUG << "Activated trigger.\n";
			});
		this->Closed([weak_this](auto&&, auto&& args) {
			auto self = weak_this.get();
			if (!self) return;
			self->m_hoverTimer.Stop();
			});
		DispatcherQueue().TryEnqueue([weak_this]()
			{
				auto self = weak_this.get();
				if (!self) return;
				self->urlTabView().SelectionChanged([self](IInspectable const&, SelectionChangedEventArgs const&) 
					{
						auto item = self->urlTabView().SelectedItem().as<TabViewItem>();
						uint32_t id = 0;
						auto items = self->urlTabView().TabItems();
						items.IndexOf(item, id);
						if (item && item.Header().try_as<hstring>() != L"正在加载中...") {
							auto value = winrt::unbox_value<uint32_t>(item.Tag());
							self->tabSeletedChangedEvent.Invoke(value);
							LOG_INFO << "SelectionChanged Index=" << id << "\n";
						}
					});
				self->urlTabView().TabCloseRequested([self](winrt::Microsoft::UI::Xaml::Controls::TabView const& sender,
					winrt::Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args) {
							self->m_isClosingTab = true;
							auto items = sender.TabItems();
							auto tab = args.Tab(); // ✔ 关键：要拿关闭的 Tab
							if (items.Size() == 1) return;//只有一个不关闭
							uint32_t index = 0;

							if (items.IndexOf(tab, index))
							{
								auto value = winrt::unbox_value<uint32_t>(tab.Tag());
								self->tabCloseEvent.Invoke(value);
								items.RemoveAt(index);
							}
					});
				
				self->UrlBox().QuerySubmitted([self](Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender,
					Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args)
					{
							auto tabView = self->urlTabView();

							// 创建新 TabViewItem
							TabViewItem newTab;
							newTab.IsClosable(false);
							newTab.Tag(box_value(self->m_nextId));
							ToolTipService::SetToolTip(
								newTab,
								box_value(L"正在加载中...")
							);
							newTab.Header(box_value(L"正在加载中..."));
							self->SiteIcon().Symbol(Microsoft::UI::Xaml::Controls::Symbol::Sync);
							tabView.TabItems().Append(newTab);
							// 获取输入的文本
							auto url = sender.Text();
							self->newUrlEnterEvent.Invoke({ self->m_nextId++, L"", url.c_str() });
					});
				self->urlTabView().AddTabButtonClick([self](IInspectable const& sender, auto const&) {
						auto tabView = self->urlTabView();

						// 创建新 TabViewItem
						TabViewItem newTab;
						newTab.IsClosable(false);
						ToolTipService::SetToolTip(
							newTab,
							box_value(L"正在加载中...")
						);
						newTab.Tag(box_value(self->m_nextId));
						newTab.Header(box_value(L"正在加载中..."));
						self->SiteIcon().Symbol(Microsoft::UI::Xaml::Controls::Symbol::Sync);

						// 添加并选中
						tabView.TabItems().Append(newTab);
						//tabView.SelectedItem(newTab);
						//默认导航网页
						std::wstring defaultUrl = L"https://www.bilibili.com/";
						self->newUrlEnterEvent.Invoke({ self->m_nextId++, L"", defaultUrl.c_str() });
					});
			});
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
	}

	void ControlWindow::InitializeControl(HWND hwnd)
	{
		if (hwnd == nullptr) {
			LOG_INFO << "InitializeControl hwnd is null.\n";
			return;
		}
		this->m_hwnd = hwnd;
		
		DispatcherQueue().TryEnqueue([self = get_strong(), hwnd]()
			{
				if (!self) return;
				//去掉标题栏和边框
				Win32Helper::RemoveFrame(hwnd);
				self->ExtendsContentIntoTitleBar(true);
				TabViewItem newTab;
				newTab.IsClosable(false);
				newTab.Tag(box_value(self->m_nextId++));
				newTab.Header(box_value(L"正在加载中..."));
				self->SiteIcon().Symbol(Microsoft::UI::Xaml::Controls::Symbol::Sync);
				self->urlTabView().TabItems().Append(newTab);
				auto  controlData = AppDataService::Get().LoadControlData();
				self->ApplyWindowState(controlData);
				//设置标题栏拖动区域
				if (auto appWin = self->AppWindow()) {
					UINT dpi = GetDpiForWindow(self->m_hwnd);
					int width = ExpandWidth * dpi / 96.0f;
					int height = ExpandHeight * dpi / 96.0f;
					appWin.MoveAndResize({ controlData.x, controlData.y, width, height });
					auto side = self->CheckDockSide({ controlData.x, controlData.y, width, height}, self->m_screenCache);
					if (side != DockSide::None)
						self->m_hoverTimer.Start();

					//appWin.Resize({ width, height });
					appWin.TitleBar().SetDragRectangles({ winrt::Windows::Graphics::RectInt32{0, 0, 10000, 40} });
					appWin.IsShownInSwitchers(false);//不出现在系统任务列表
				}
				if (auto presenter = self->AppWindow().Presenter().as<OverlappedPresenter>())
				{
					presenter.IsAlwaysOnTop(true);
					//presenter.SetBorderAndTitleBar(false, false);
					presenter.IsResizable(false);
					presenter.IsMaximizable(false);
					presenter.IsMinimizable(false);
				}
				self->m_screenCache = self->GetScreenWorkArea();

				//过滤该窗口自身按键消息
				PipeService::Get().SendFilterRule(hwnd);
			});
	}

	DockSide ControlWindow::CheckDockSide(const RectInt32& windowBounds, const RectInt32& screen) noexcept
	{
		if (windowBounds.X <= AdsorbThreshold)
			return DockSide::Left;

		if (screen.Width - (windowBounds.X + windowBounds.Width) <= AdsorbThreshold)
			return DockSide::Right;

		if (windowBounds.Y <= AdsorbThreshold)
			return DockSide::Top;

		return DockSide::None;
	}

	void ControlWindow::Grid_PointerPressed(IInspectable const& sender, Input::PointerRoutedEventArgs const& e)
	{
		m_isDragging = true;
		m_userInteracted = true;
		auto pt = e.GetCurrentPoint(nullptr);

		//m_dragStartMouse = pt.Position();

		GetCursorPos(&m_dragStartCursor);

		m_dragStartWindowPos = AppWindow().Position();

		auto element = sender.try_as<UIElement>();
		if (element)
			element.CapturePointer(e.Pointer());
	}

	void ControlWindow::Grid_PointerMoved(IInspectable const& sender, Input::PointerRoutedEventArgs const& e)
	{
		if (!m_isDragging) return;

		using namespace std::chrono;

		auto now = steady_clock::now();
		// 每帧间隔
		auto frameTime = milliseconds(1000 / m_refreshRate);
		// ⭐ 限流：16ms ≈ 60fps
		if (now - m_lastMoveTime < frameTime)
			return;

		m_lastMoveTime = now;

		POINT currentPt;
		GetCursorPos(&currentPt);
		int dx = currentPt.x - m_dragStartCursor.x;
		int dy = currentPt.y - m_dragStartCursor.y;

		int newX = m_dragStartWindowPos.X + dx;
		int newY = m_dragStartWindowPos.Y + dy;

		// ⭐ 只 Move，不做任何 GetWindowRect / DisplayArea
		AppWindow().Move({ newX, newY });
	}

	void ControlWindow::Grid_PointerReleased(IInspectable const& sender, Input::PointerRoutedEventArgs const& e)
	{
		m_isDragging = false;

		auto element = sender.try_as<UIElement>();
		if (element)
			element.ReleasePointerCapture(e.Pointer());

		auto winPos = AppWindow().Position();
		auto winSize = AppWindow().Size();

		RectInt32 rec{
			winPos.X, winPos.Y,
			winSize.Width, winSize.Height
		};

		auto side = CheckDockSide(rec, m_screenCache);
		UINT dpi = GetDpiForWindow(m_hwnd);
		int dockSnapWidth = DockSnapWidth * dpi / 96.0f;
		int dockSnapHeight = DockSnapHeight * dpi / 96.0f;
		int expandWidth = ExpandWidth * dpi / 96.0f;
		int expandHeight = ExpandHeight * dpi / 96.0f;
		switch (side)
		{
		case DockSide::Left:
			AppWindow().MoveAndResize({ 0, rec.Y, dockSnapWidth, expandHeight });
			break;

		case DockSide::Right:
			AppWindow().MoveAndResize({
				m_screenCache.Width - dockSnapWidth,
				rec.Y,
				dockSnapWidth,
				expandHeight });
			break;

		case DockSide::Top:
			AppWindow().MoveAndResize({ rec.X, 0, expandWidth, dockSnapHeight });
			break;
		}
	}

	void ControlWindow::Grid_PointerEntered(IInspectable const&, Input::PointerRoutedEventArgs const&)
	{
		auto bounds = GetWindowRect();
#ifdef _DEBUG
		std::string str{ "" };
		str += "(X, Y, Width, Height)=(" +
			std::to_string(bounds.X) +
			", " + std::to_string(bounds.Y) +
			", " + std::to_string(bounds.Width) +
			", " + std::to_string(bounds.Height) +
			")\n";
		LOG_DEBUG << str;
#endif
		m_isEntered = true;
		auto size = CheckDockSide(bounds, m_screenCache);
		if (m_pendingResize)
			return;

		m_pendingResize = true;

		DispatcherQueue().TryEnqueue([this, bounds, size]()
			{
				m_pendingResize = false;
				UINT dpi = GetDpiForWindow(m_hwnd);
				int width = ExpandWidth *  dpi / 96.0f;
				int height = ExpandHeight * dpi / 96.0f;
				int dockSnapWidth = DockSnapWidth * dpi / 96.0f;
				int dockSnapHeight = DockSnapHeight * dpi / 96.0f;
				// 判断当前是收缩态，还原展开尺寸
				if (bounds.Width <= dockSnapWidth || bounds.Height <= dockSnapHeight)
				{
					if (size == DockSide::Right)
					{
						AppWindow().MoveAndResize({
							m_screenCache.Width - width,
							bounds.Y,
						    width,
							height
							});
					}
					else
					{
						AppWindow().MoveAndResize({
							bounds.X,
							bounds.Y,
							width,
							height
							});
					}
				}
			});
	}

	void ControlWindow::Grid_PointerExited(IInspectable const&, Input::PointerRoutedEventArgs const&)
	{
		static POINT currentPt;
		::GetCursorPos(&currentPt);
		if (!IsPointInsideWindow(currentPt))
			m_hoverTimer.Start();
		m_isEntered = false;
		LOG_DEBUG << "Grid_PointerExited trigger.\n";
	}

	bool ControlWindow::IsPointInsideWindow(POINT& pt)
	{
		auto pos = AppWindow().Position();
		auto size = AppWindow().Size();

		int left = pos.X;
		int top = pos.Y;
		int right = pos.X + size.Width;
		int bottom = pos.Y + size.Height;

		return (pt.x >= left &&
			pt.x <= right &&
			pt.y >= top &&
			pt.y <= bottom);
	}
	void ControlWindow::SetVisibleInvoker(Event<bool>& event)
	{
		auto weak_this = this->get_weak();
		visibleInvoker = event(winrt::auto_revoke, [weak_this](bool isVisible)//->
			//winrt::fire_and_forget
			{
				//winrt::resume_background();
				if (auto self = weak_this.get()) {
					if (isVisible/* && !self->m_isActive*/)
					{
						//self->AppWindow().Show();
						self->m_isActive = true;
						::ShowWindowAsync(self->m_hwnd, SW_SHOWNOACTIVATE);//使用异步show否则会阻塞主窗口resize

						LOG_DEBUG << L"[ControlWindow]接收到窗口显示信号\n";
						//self->m_hideTimer.Start();
					}
					else if (/*self->m_isActive &&*/ !self->m_isEntered)
					{
						//self->AppWindow().Hide();
						self->m_isActive = false;
						::ShowWindowAsync(self->m_hwnd, SW_HIDE);

						//self->AppWindow().IsShownInSwitchers();
						LOG_DEBUG << L"[ControlWindow]接收到窗口隐藏信号\n";
					}
				}
				return;
			});
	}

	void ControlWindow::SetCloseEventInvoker(Event<bool>& event)
	{
		auto weak_this = this->get_weak();
		closeEventRevoker = event(auto_revoke, [weak_this](bool isClose) {
			if (auto self = weak_this.get())
			{
				auto dispatcher = self->DispatcherQueue();
				dispatcher.TryEnqueue([weak_this]() {
					if (auto self = weak_this.get())
					{
						self->Close(); // ✔ 延迟执行，安全
					}
					});
			}
			});
	}
	void ControlWindow::SetPageCreatedStateEventRevoker(Event<TabInfo>& event)
	{
		std::lock_guard lock(m_mutex);
		auto weak_this = this->get_weak();
		pageCreatedStateEventRevoker = event(auto_revoke, [weak_this](TabInfo info) {
			
			if (auto self = weak_this.get())
			{
				auto items = self->urlTabView().TabItems();
				
				if (auto item = self->FindTabViewItem(info.idx)) {

					item.IsClosable(true);
					item.Header(box_value(info.title));
					ToolTipService::SetToolTip(
						item,
						box_value(info.title)
					);
					//self->TabViewTitle(info.title.c_str());
					self->UrlBox().Text(info.url);
					self->urlTabView().SelectedItem(item);
					self->SiteIcon().Symbol(Microsoft::UI::Xaml::Controls::Symbol::Globe);
				}
			}
			});
	}

	void ControlWindow::HandleEvent(UINT msg) noexcept
	{
		auto weak_this = this->get_weak();
		switch (msg)
		{
		case WM_IncreaseOpacity:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			BYTE alpha = Win32Helper::GetOpacity(m_hwnd) - 10;
			Win32Helper::SetOpacity(m_hwnd, alpha);
			break;
		}
		case WM_DecreaseOpacity:
		{
			if (!AppDataService::Get().HotkeyEnableState())
				break;
			byte alpha = Win32Helper::GetOpacity(m_hwnd) + 10;
			Win32Helper::SetOpacity(m_hwnd, alpha);
			break;
		}
		case WM_EnableHotkeys:
		{
			AppDataService::Get().ToggleHotkeysEnabled();
			LOG_INFO << "Hotkeys enabled: " << AppDataService::Get().HotkeyEnableState() << "\n";
			break;
		}
		default:
			break;
		}
	}

	void ControlWindow::ApplyWindowState(const ControlWindowData& state) noexcept
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

	void ControlWindow::SaveWindowStateData() noexcept
	{
		if (!m_hwnd)
		{
			LOG_INFO << "SaveWindowStateData read m_hwnd is nullptr \n";
			return;
		}

		ControlWindowData state;
		//先获取窗口的大小和位置


		RECT rc{};
		::GetWindowRect(m_hwnd, &rc);

		state.x = rc.left;
		state.y = rc.top;
		state.width = rc.right - rc.left;
		state.height = rc.bottom - rc.top;

		//获取窗的最大最小化状态
		WINDOWPLACEMENT wp{ sizeof(wp) };
		GetWindowPlacement(m_hwnd, &wp);
		//窗口的透明度
		state.alpha = Win32Helper::GetOpacity(m_hwnd);


		//热键在后台service进程里写
		AppDataService::Get().SaveControlData(state);

	}

	TabViewItem ControlWindow::FindTabViewItem(uint32_t idx)
	{
		for (const auto& item : this->urlTabView().TabItems())
		{
			auto tab = item.try_as<TabViewItem>();
			if (!tab) continue;

			auto tag = tab.Tag();

			if (tag)
			{
				if (winrt::unbox_value<uint32_t>(tag) == idx)
				{
					return tab;
				}
			}
		}

		return nullptr;
	}

	void ControlWindow::SetPipeServiceHandleEvent(Event<UINT>& event)
	{
		m_pipeServiceHandleRevoker = event(auto_revoke, [this](UINT msg) {
			this->HandleEvent(msg);
		});
	}

	void ControlWindow::SetSystemTrayClickEventRevoker(Event<>& event)
	{
		m_systemTrayClickEventRevoker = event(auto_revoke, [this]() {
			AppWindow().Show();
		});
	}

	void ControlWindow::SetSystemTrayShowWindowRevoker(Event<>& event)
	{
		m_systemTrayShowWindowRevoker = event(auto_revoke, [this]() {
			AppWindow().Show();
			});
	}
}

