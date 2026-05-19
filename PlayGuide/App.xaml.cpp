#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "ControlWindow.xaml.h"
#include "PipeService.h"
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

#include "LocalizationHelper.h"
#include "ThemeService.h"
#include "Global.h"
#include "TrayIconService.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using winrt::Microsoft::UI::Xaml::Media::MicaBackdrop;
using winrt::Microsoft::UI::Xaml::Media::DesktopAcrylicBackdrop;

namespace winrt::PlayGuide::implementation
{
	/// <summary>
	/// Initializes the singleton application object.  This is the first line of authored code
	/// executed, and as such is the logical equivalent of main() or WinMain().
	/// </summary>
	App::App()
	{
		// Xaml objects should not call InitializeComponent during construction.
		// See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
		UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
			{
				if (IsDebuggerPresent())
				{
					auto errorMessage = e.Message();
					__debugbreak();
				}
			});

#endif
	}
	HWND App::GetHwnd(winrt::Microsoft::UI::Xaml::Window const& window)
	{
		HWND hwnd{ nullptr };
		auto windowNative = window.as<IWindowNative>();
		// 等到 HWND 真正创建
		for (int i = 0; i < 50; i++)
		{
			windowNative->get_WindowHandle(&hwnd);

			if (hwnd != nullptr)
				break;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		return hwnd;
	}
	/// <summary>
	/// Invoked when the application is launched.
	/// </summary>
	/// <param name="e">Details about the launch request and process.</param>
	void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
	{
		auto weak_this = get_weak();
		//本地化服务
		LocalizationHelper::Get().Initialize();
		m_controlWindow = make<ControlWindow>();
		auto controlWindow = m_controlWindow.try_as<ControlWindow>();

		m_mainWindow = make<MainWindow>(L"https://www.bilibili.com");
		auto mainWindow = m_mainWindow.try_as<MainWindow>();
		//注册事件
		controlWindow->SetVisibleInvoker(mainWindow->controlWindowVisible);
		mainWindow->SetTabCloseEvent(controlWindow->tabCloseEvent);
		mainWindow->SetTabSeletedChangedEvent(controlWindow->tabSeletedChangedEvent);
		mainWindow->SetNewUrlEnterEvent(controlWindow->newUrlEnterEvent);
		controlWindow->SetPageCreatedStateEventRevoker(mainWindow->pageCreatedStateEvent);

		PipeService::Get().SetHotkeyMsgHandler([weak_this](UINT msg) {
			if (auto self = weak_this.get())
				g_pipeServiceHandleEvent.Invoke(msg);
			});

		mainWindow->SetPipeServiceHandleEvent(g_pipeServiceHandleEvent);
		controlWindow->SetPipeServiceHandleEvent(g_pipeServiceHandleEvent);

		mainWindow->SetSystemTrayClickEventRevoker(TrayIconService::Get().LeftClickEvent);
		controlWindow->SetSystemTrayClickEventRevoker(TrayIconService::Get().LeftClickEvent);

		mainWindow->SetSystemTrayShowWindowRevoker(TrayIconService::Get().ShowMainWindowEvent);
		controlWindow->SetSystemTrayShowWindowRevoker(TrayIconService::Get().ShowControlWindowEvent);

		// 注册结束程序事件
		processExitEventRevoker = g_processExitEvent(auto_revoke, [weak_this]()
			{
				if (auto self = weak_this.get()) {
					auto controlWindow = self->m_controlWindow.try_as<ControlWindow>();
					auto mainWindow = self->m_mainWindow.try_as<MainWindow>();
					mainWindow->SaveWindowStateData();
					controlWindow->SaveWindowStateData();
					TrayIconService::Get().Cleanup();

					self->m_controlWindow.Close();
					self->m_mainWindow.Close();
					// 回调函数变成无资格协程，切到后台或等待当前调用栈销毁后再执行退出
					//PipeService::Get().Stop();

					//co_await winrt::resume_after(std::chrono::milliseconds(200));
					//co_await winrt::resume_after(std::chrono::milliseconds(100));
					//self->Exit();
					PipeService::Get().Stop();
					::PostQuitMessage(0);
					//co_return;
				}
			});

		//主题服务
		ThemeService::RegisterWindow(m_controlWindow);
		ThemeService::RegisterWindow(m_mainWindow);
		auto theme = AppDataService::Get().Theme();
		ThemeService::SetTheme(theme);


		mainWindow->MainInitialize(GetHwnd(m_mainWindow));
		controlWindow->InitializeControl(GetHwnd(m_controlWindow));
	}
}