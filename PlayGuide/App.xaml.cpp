#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "ControlWindow.xaml.h"
#include "PipeService.h"
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

#include "LocalizationHelper.h"
#include "ThemeService.h"

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
		LocalizationHelper::Get().Initialize();
		//LocalizationHelper::Get().SetLanguage(L"en-US");
		m_controlWindow = make<ControlWindow>();
		auto controlWindow = m_controlWindow.try_as<ControlWindow>();
		controlWindow->InitializeControl(GetHwnd(m_controlWindow));

		m_controlWindow.DispatcherQueue().TryEnqueue([self = get_strong(), controlWindow]() {
			self->m_mainWindow = make<MainWindow>(L"https://www.bilibili.com");
			auto mainWindow = self->m_mainWindow.try_as<MainWindow>();
			self->closeControlWindowEvent = mainWindow->controlWindowCloseEvent(auto_revoke, [self](bool value) {
				self->m_controlWindow.Close();
				//winrt::resume_after(std::chrono::milliseconds(1000));
				self->Exit();
				});
			controlWindow->SetVisibleInvoker(mainWindow->controlWindowVisible);
			mainWindow->SetTabCloseEvent(controlWindow->tabCloseEvent);
			mainWindow->SetTabSeletedChangedEvent(controlWindow->tabSeletedChangedEvent);
			mainWindow->SetNewUrlEnterEvent(controlWindow->newUrlEnterEvent);
			controlWindow->SetPageCreatedStateEventRevoker(mainWindow->pageCreatedStateEvent);
			mainWindow->MainInitialize(self->GetHwnd(self->m_mainWindow));

			PipeService::Get().SetHotkeyMsgHandler([self](UINT msg) {
				self->pipeServiceHandleEvent.Invoke(msg);
				});
			
			mainWindow->SetPipeServiceHandleEvent(self->pipeServiceHandleEvent);
			controlWindow->SetPipeServiceHandleEvent(self->pipeServiceHandleEvent);

			ThemeService::RegisterWindow(self->m_controlWindow);
			ThemeService::RegisterWindow(self->m_mainWindow);
			auto theme = AppDataService::Get().Theme();
			ThemeService::SetTheme(theme);
			self->m_controlWindow.Activate();
			self->m_mainWindow.Activate();
		 });
	}
}