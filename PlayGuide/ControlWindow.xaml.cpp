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
#include "Loc.h"
#include "global.h"
#include "LocalizationHelper.h"

#include "AboutWindow.xaml.h"

using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Windowing;
using namespace Windows::Graphics;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Input;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

static PlayGuide::AboutWindow g_aboutWindow{ nullptr };

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
				if (!self->m_isActive)
					self->m_hideTimer.Start();
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
				self->urlTabView().SelectionChanged([self](IInspectable const&, SelectionChangedEventArgs const&args) 
					{
						if (args.AddedItems().Size() == 0)
							return;

						auto item =
							args.AddedItems().GetAt(0).try_as<TabViewItem>();

						if (!item)
							return;

						auto tag = item.Tag();

						if (!tag)
							return;

						auto data = tag.as<IPropertySet>();
						auto idx = unbox_value<uint32_t>(data.Lookup(L"idx"));
						auto url = unbox_value<hstring>(data.Lookup(L"url"));
						self->UrlBox().Text(url);

						self->tabSeletedChangedEvent.Invoke(TabInfo{ idx, L"", url.c_str() });

						LOG_INFO << "SelectionChanged Index=" << idx << "\n";
					});

				self->urlTabView().TabCloseRequested([self](winrt::Microsoft::UI::Xaml::Controls::TabView const& sender,
					winrt::Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args) {
							self->m_isClosingTab = true;
							auto items = sender.TabItems();
							auto tab = args.Tab(); //拿关闭的 Tab

							uint32_t index= 0;
							if (!items.IndexOf(tab, index))
								return;

							
							// 删除前决定新的选中目标
							TabViewItem nextTab{ nullptr };

							if (items.Size() > 1)
							{
								// 优先选右边
								if (index + 1 < items.Size())
								{
									nextTab =
										items.GetAt(index + 1).as<TabViewItem>();
								}
								else
								{
									// 否则选左边
									nextTab =
										items.GetAt(index - 1).as<TabViewItem>();
								}
							}
							// 删除当前 tab
							items.RemoveAt(index);

							auto data = tab.Tag().as<IPropertySet>();
							auto idx = unbox_value<uint32_t>(data.Lookup(L"idx"));

							self->tabCloseEvent.Invoke(idx);
							// 手动切换
							if (nextTab)
							{
								self->urlTabView().SelectedItem(nextTab);
							}
							else {
								self->tabSeletedChangedEvent.Invoke(TabInfo{ 65535, L"", L"" });
							}
					});
				
				self->UrlBox().QuerySubmitted([self](Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender,
					Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args)
					{
							auto tabView = self->urlTabView();
							auto url = sender.Text();
							//补全协议
							if (!url.starts_with(L"http://") &&
								!url.starts_with(L"https://"))
							{
								url = L"https://" + url;
							}

							if (!self->IsProbablyUrl(url.c_str()))
							{
								self->UrlBox().Text(LocalizationHelper::Get().String(L"IsNotProbablyUrl"));
								return;
							}
							// 创建新 TabViewItem
							auto newTab = self->CreateTabItem(LocalizationHelper::Get().String(L"Loading"), nullptr);
							newTab.IsClosable(false);
							PropertySet data;
							data.Insert(L"url", box_value(url));
							data.Insert(L"title", box_value(L""));
							data.Insert(L"idx", box_value(self->m_nextId++));
							newTab.Tag(box_value(data));

							ToolTipService::SetToolTip(
								newTab,
								box_value(LocalizationHelper::Get().String(L"Loading"))
							);
						
							//self->SiteIcon().Symbol(Microsoft::UI::Xaml::Controls::Symbol::Sync);
							tabView.TabItems().Append(newTab);
							tabView.SelectedItem(newTab);
							// 获取输入的文本
							//self->newUrlRequestEvent.Invoke({ self->m_nextId++, L"", url.c_str() });
					});
				self->urlTabView().AddTabButtonClick([self](IInspectable const& sender, auto const&) {
						auto tabView = self->urlTabView();

						// 创建新 TabViewItem
						auto newTab = self->CreateTabItem(LocalizationHelper::Get().String(L"Loading"), nullptr);
						newTab.IsClosable(false);
						ToolTipService::SetToolTip(
							newTab,
							box_value(LocalizationHelper::Get().String(L"Loading"))
						);
						PropertySet data;
						//默认导航网页
						hstring defaultUrl = AppSettingsViewModel::Instance().HomePage();
						data.Insert(L"url", box_value(defaultUrl));
						data.Insert(L"title", box_value(L""));
						data.Insert(L"idx", box_value(self->m_nextId++));
						newTab.Tag(box_value(data));

						//self->SiteIcon().Symbol(Microsoft::UI::Xaml::Controls::Symbol::Sync);

						// 添加并选中
						tabView.TabItems().Append(newTab);
						tabView.SelectedItem(newTab);
						
						//self->newUrlRequestEvent.Invoke({ self->m_nextId++, L"", defaultUrl.c_str() });
					});
				
			});
		m_languageChangedEventRevoker = g_languageChanged(auto_revoke, [weak_this]() {
			if (auto self = weak_this.get()) {
				self->About().Text(LocalizationHelper::Get().String(L"About"));
				self->Setting().Text(LocalizationHelper::Get().String(L"Settings"));
			}
			});
		m_hotkeyChangedEventRevoker = g_hotkeyChanged(auto_revoke, [weak_this](const std::wstring& id) {
			if (auto self = weak_this.get()) {
				auto vm = AppSettingsViewModel::Instance().try_as<PlayGuide::AppSettingsViewModel>();

				auto source = self->HotkeyLabels().ItemsSource();

				auto hotkeys =
					source.as<winrt::Windows::Foundation::Collections::IVector<
					winrt::Windows::Foundation::IInspectable>>();

				for (uint32_t i = 0; i < hotkeys.Size(); i++)
				{
					auto item = hotkeys.GetAt(i);

					auto hk = item.as<PlayGuide::HotkeyItemViewModel>();

					if (hk.id() == id)
					{
						auto value = make<HotkeyItemViewModel>(hk.Name(), hk.Description(), hk.Key(), hk.IconGlyph(), hk.id());
						hotkeys.SetAt(i, value);
						break;
					}
				}
			}
			});

		m_newWindowRequestedEventRevoker = g_newWindowRequested(auto_revoke, [weak_this](const TabInfo& info) {
			if (auto self = weak_this.get())
			{
				auto tabView = self->urlTabView();

				// 创建新 TabViewItem
				auto newTab = self->CreateTabItem(LocalizationHelper::Get().String(L"Loading"), nullptr);
				newTab.IsClosable(false);
				PropertySet data;
				data.Insert(L"url", box_value(info.url));
				data.Insert(L"title", box_value(info.title));
				data.Insert(L"idx", box_value(self->m_nextId++));
				newTab.Tag(box_value(data));
			   

				tabView.TabItems().Append(newTab);
				tabView.SelectedItem(newTab);

				//self->newUrlRequestEvent.Invoke({ self->m_nextId++, L"", info.url });
			}
			});


		m_documentTitleChangedEventRevoker = g_documentTitleChanged(auto_revoke, [weak_this](const TabInfo& info) {
			if (auto self = weak_this.get())
			{
				if (auto item = self->FindTabViewItem(info.idx)) {

					item.IsClosable(true);
					auto grid = item.Header().try_as<Grid>();
					auto header = self->GetHeader(grid);
					header.title.Text(info.title);
					if (auto data = item.Tag().try_as<IPropertySet>())
					{
						data.Insert(L"title", box_value(info.title));
					}
				  
					ToolTipService::SetToolTip(
						item,
						box_value(info.title)
					);
				}
			}
			});
		m_sourceChangedEventRevoker = g_sourceChanged(auto_revoke, [weak_this](const TabInfo& info) {
			if (auto self = weak_this.get())
			{
				if (auto item = self->FindTabViewItem(info.idx)) {
					self->UrlBox().Text(info.url);
					ToolTipService::SetToolTip(
						self->UrlBox(),
						box_value(info.url)
					);
					auto tag = item.Tag();
					auto data = tag.as<IPropertySet>();
					data.Insert(L"url", box_value(info.url));
				}
			}
			});

		m_navigationStartingEventRevoker = g_navigationStarting(auto_revoke, [weak_this](const TabInfo& info) {
			if (auto self = weak_this.get())
			{
				if (auto item = self->FindTabViewItem(info.idx)) {
					item.IsClosable(true);
					auto grid = item.Header().try_as<Grid>();
					TabHeaderView header = self->GetHeader(grid);
					header.title.Text(LocalizationHelper::Get().String(L"Loading"));
				}
			}
			});
		m_hideTimer = dispatcherQueue.CreateTimer();
		m_hideTimer.Interval(std::chrono::milliseconds(3000)); // 延迟时间
		m_hideTimer.IsRepeating(false); // 关键：只执行一次
		m_hideTimer.Tick([weak_this](auto&&, auto&&)
		{
				if (auto self = weak_this.get())
				{
					auto winBounds = self->GetWindowRect();
					auto side = self->CheckDockSide(winBounds, self->m_screenCache);
					
					if(side != DockSide::None)
                        self->AppWindow().Hide();
				}
		});

		m_faviconChanged = g_faviconChanged(auto_revoke, [weak_this](const TabInfoEx& info) {
			if (auto self = weak_this.get())
			{
				if (auto item = self->FindTabViewItem(info.idx))
				{
					auto grid = item.Header().try_as<Grid>();
					auto header = self->GetHeader(grid);
					header.favicon.Source(info.favicon);
					header.favicon.Visibility(Visibility::Visible);
				}
			}
			});

		m_isDocumentPlayingAudio = g_isDocumentPlayingAudio(auto_revoke, [weak_this](const TabInfoEx& info) {
			if (auto self = weak_this.get())
			{
				if (auto item = self->FindTabViewItem(info.idx))
				{
					auto grid = item.Header().try_as<Grid>();
					auto header = self->GetHeader(grid);
					header.status.Glyph(info.isPlayingAudio ? L"\uE767" : L"\uE74F");
					header.status.Visibility(Visibility::Visible);
				}
			}
			});
		
		m_showSettingsPageEventRevoker = g_showSettingsPageEvent(auto_revoke, [weak_this]() {
			if (auto self = weak_this.get())
			{
				self->SettingsButton_Clicked(nullptr, nullptr);
			}
			});
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

				self->About().Text(LocalizationHelper::Get().String(L"About"));
				self->Setting().Text(LocalizationHelper::Get().String(L"Settings"));
				auto vm = AppSettingsViewModel::Instance().try_as<PlayGuide::AppSettingsViewModel>();
				self->HotkeyLabels().ItemsSource(vm.Hotkeys());

				//恢复创建上次打开的url
				self->RestoreTabItems(controlData.urls);
				//恢复选中
				auto tabItems = self->urlTabView().TabItems();
				if (tabItems && controlData.selectedItem >= 0 && controlData.selectedItem < tabItems.Size())
				{
					self->urlTabView().SelectedIndex(controlData.selectedItem);
				}
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
		//m_hideTimer.Start();
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
						self->Close();
					}
					});
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
		case WM_ShowHideWindow:
		{
			m_hideTimer.Start();
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

		do
		{
			auto tabItems = urlTabView().TabItems();

			if (!tabItems) break;

			for (int i = 0; i < tabItems.Size(); ++i)
			{
				if (auto item = tabItems.GetAt(i).try_as<TabViewItem>())
				{
					auto tag = item.Tag();
					if (auto data = tag.try_as<IPropertySet>())
					{
						auto url = unbox_value<hstring>(data.Lookup(L"url"));
						auto title = unbox_value<hstring>(data.Lookup(L"title"));
						if (url != L"Settings")
						{
							state.urls.emplace_back(TabInfo{ 0, title.c_str(), url.c_str() });
						}
					}
				}
			}
			auto selectedIdx = urlTabView().SelectedIndex();
			if (auto item = tabItems.GetAt(selectedIdx).try_as<TabViewItem>())
			{
				auto tag = item.Tag();
				if (auto data = tag.try_as<IPropertySet>())
				{
					auto idx = unbox_value<uint32_t>(data.Lookup(L"idx"));
					if (idx != 0)
					{
						state.selectedItem = selectedIdx;
					}
					else state.selectedItem = tabItems.Size() - 1;
				}
			}

		} while (false);
		AppDataService::Get().SaveControlData(state);

	}

	TabViewItem ControlWindow::FindTabViewItem(uint32_t idx)
	{
        for (int i = 0; i < this->urlTabView().TabItems().Size(); ++i)
        {
            auto item = this->urlTabView().TabItems().GetAt(i);
            auto tab = item.try_as<TabViewItem>();
            if (!tab) continue;
            if (auto tag = tab.Tag())
            {
                auto data = tag.as<IPropertySet>();
                auto itemIdx = unbox_value<uint32_t>(data.Lookup(L"idx"));
                if (itemIdx == idx)
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
		m_systemTrayShowWindowRevoker = event(auto_revoke, [weak_this = this->get_weak()]() {
			if (auto self = weak_this.get()) {
				auto winBounds = self->GetWindowRect();
				auto side = self->CheckDockSide(winBounds, self->m_screenCache);
				if (side != DockSide::None)
				{
					UINT dpi = GetDpiForWindow(self->m_hwnd);
					int expandWidth = ExpandWidth * dpi / 96.0f;
					int expandHeight = ExpandHeight * dpi / 96.0f;
					self->AppWindow().Resize({ expandWidth, expandHeight });
					self->m_hoverTimer.Start();
				}
				self->AppWindow().Show();
			}
			});
	}

	void ControlWindow::SettingsButton_Clicked(IInspectable const&, RoutedEventArgs const&)
	{
		//遍历tabviewitems，查看是否已经打开设置页面，如果没有打开就新建tabviewitem
		auto tabView = urlTabView();
		// 1. 先检查是否已经打开设置页
		for (auto const& obj : tabView.TabItems())
		{
			auto tab = obj.try_as<Microsoft::UI::Xaml::Controls::TabViewItem>();

			if (!tab)
				continue;

			// 通过 Tag 判断
			auto tag = tab.Tag();

			if (tag)
			{
				auto data = winrt::unbox_value<IPropertySet>(tag);
				auto idx = winrt::unbox_value<uint32_t>(data.Lookup(L"idx"));
				if (idx == 0)
				{
					// 已存在 -> 直接切换
					tabView.SelectedItem(tab);
					return;
				}
			}
		}
		// 2. 不存在则创建 Settings Tab
		Microsoft::UI::Xaml::Controls::TabViewItem settingsTab;

		settingsTab.IsClosable(true);

		settingsTab.Header(box_value(LocalizationHelper::Get().String(L"Settings")));
		PropertySet data;
		data.Insert(L"url", box_value(L"Settings"));
		data.Insert(L"title", box_value(L"Settings"));
		data.Insert(L"idx", box_value(uint32_t(0)));

		settingsTab.Tag(box_value(data));

		ToolTipService::SetToolTip(
			settingsTab,
			box_value(LocalizationHelper::Get().String(L"Settings"))
		);

		// 设置图标
		 // ===== Header =====

		Microsoft::UI::Xaml::Controls::StackPanel headerPanel;
		headerPanel.Orientation(
			Microsoft::UI::Xaml::Controls::Orientation::Horizontal);

		headerPanel.Spacing(6);

		// 图标
		Microsoft::UI::Xaml::Controls::FontIcon icon;
		icon.Glyph(L"\uE713"); // 设置图标

		// 文本
		Microsoft::UI::Xaml::Controls::TextBlock text;
		text.Text(LocalizationHelper::Get().String(L"Settings"));

		headerPanel.Children().Append(icon);
		headerPanel.Children().Append(text);

		settingsTab.Header(headerPanel);

		// 添加并选中
		tabView.TabItems().Append(settingsTab);

		tabView.SelectedItem(settingsTab);

	}
	void ControlWindow::AboutButton_Clicked(IInspectable const&, RoutedEventArgs const&)
	{
		if (!g_aboutWindow)
		{
			g_aboutWindow = PlayGuide::AboutWindow();

			g_aboutWindow.Closed([](auto&&, auto&&)
				{
					g_aboutWindow = nullptr;
				});
		}

		g_aboutWindow.Activate();
	}

	void ControlWindow::HotkeyLabels_SizeChanged(
		winrt::Windows::Foundation::IInspectable const&,
		winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e)
	{
		
		// 1. 获取 ListView 当前的总可用宽度
     	double totalWidth = e.NewSize().Width;

		// 2. 获取绑定的快捷键列表数量
		auto items = HotkeyLabels().ItemsSource();
		if (!items) return;

		// 假设数据源可以通过某个集合获取数量，比如总共有 count 个
		uint32_t count = ViewModel().Hotkeys().Size();

		if (count == 0) return;

		// 3. 计算每个子项应该分到的绝对平均宽度
		double averageWidth = totalWidth / count;

		// 4. 遍历当前已经渲染出来的 ListViewItem 容器，强制设置它们的宽度
		for (uint32_t i = 0; i < count; i++)
		{
			auto container = HotkeyLabels().ContainerFromIndex(i);
			if (container)
			{
				if (auto listViewItem = container.as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>())
				{
					// 迫使每个列表项均分宽度
					listViewItem.Width(averageWidth);
				}
			}
		}
		//RecalcHotkeyLayout();
	}

	void ControlWindow::RecalcHotkeyLayout()
	{
		double width = HotkeyLabels().ActualWidth();
		UINT dpi = GetDpiForWindow(m_hwnd);
		width = width * dpi / 96.f;

		auto count = ViewModel().Hotkeys().Size();

		if (count == 0) return;

		double itemWidth = width / count;
		for (uint32_t i = 0; i < count; i++)
		{
			auto container = HotkeyLabels().ContainerFromIndex(i);
			if (container)
			{
				if (auto listViewItem = container.as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>())
				{
					// 迫使每个列表项均分宽度
					listViewItem.Width(itemWidth);
				}
			}
		}
	}

	void ControlWindow::RestoreTabItems(std::vector<TabInfo> const& tabs)
	{
		using namespace winrt;
		using namespace Microsoft::UI::Xaml::Controls;
		using namespace Windows::Foundation::Collections;

		for (auto const& tab : tabs)
		{
			TabViewItem item = CreateTabItem(tab.title.c_str(), nullptr);

			// 2. Tag = PropertySet (idx + url)
			PropertySet set;
			set.Insert(L"idx", box_value(m_nextId++));
			set.Insert(L"url", box_value(tab.url));

			item.Tag(box_value(set));

			urlTabView().TabItems().Append(item);
		}
	}

	bool ControlWindow::IsProbablyUrl(std::wstring const& input)
	{
		if (input.empty())
			return false;

		// 已有协议
		if (input.starts_with(L"http://") ||
			input.starts_with(L"https://") ||
			input.starts_with(L"file://") ||
			input.starts_with(L"ms-appx://"))
		{
			return true;
		}

		// 至少包含点号（域名特征）
		if (input.find(L'.') != std::wstring::npos)
		{
			return true;
		}

		return false;
	}

	TabViewItem ControlWindow::CreateTabItem(hstring const& title,
		winrt::Microsoft::UI::Xaml::Media::ImageSource favicon)
	{
		TabViewItem item;

		Grid header;
		ColumnDefinition col1;
		col1.Width(GridLength{ 0, GridUnitType::Auto });

		ColumnDefinition col2;
		col2.Width(GridLength{ 1, GridUnitType::Star });

		ColumnDefinition col3;
		col3.Width(GridLength{ 0, GridUnitType::Auto });

		header.ColumnDefinitions().Append(col1);
		header.ColumnDefinitions().Append(col2);
		header.ColumnDefinitions().Append(col3);

		// favicon
		Image icon;
		icon.Width(16);
		icon.Height(16);
		icon.Source(favicon);
		icon.Visibility(Visibility::Collapsed);
		icon.Margin({ 0,0,8,0 });
		Grid::SetColumn(icon, 0);

		// title
		TextBlock text;
		text.Text(title);
		text.TextTrimming(TextTrimming::CharacterEllipsis);
		Grid::SetColumn(text, 1);

		// status icon
		FontIcon status;
		status.Glyph(L"\uE767");
		status.Visibility(Visibility::Collapsed);
		Grid::SetColumn(status, 2);
		status.Margin({ 8,0,0,0 });
		header.Children().Append(icon);
		header.Children().Append(text);
		header.Children().Append(status);

		item.Header(header);

		return item;
	}

}

