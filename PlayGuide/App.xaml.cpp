#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "ControlWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>

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
		/*
		// 获取当前线程的 DispatcherQueue
		
		dispatcherQueue.TryEnqueue([self = this->get_strong()]() {
			self->m_mainWindow = make<MainWindow>(L"https://www.bilibili.com");
			auto mainWindow = self->m_mainWindow.try_as<MainWindow>();
			//mainWindow->MainInitialize(self->GetHwnd(self->m_mainWindow));
			self->m_mainWindow.Activate();
			});
		*/
		//auto dispatcherQueue = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
		m_controlWindow = make<ControlWindow>();
		auto controlWindow = m_controlWindow.try_as<ControlWindow>();
		controlWindow->InitializeControl(GetHwnd(m_controlWindow));
		m_controlWindow.Activate();

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
			self->m_mainWindow.Activate();
			mainWindow->MainInitialize(self->GetHwnd(self->m_mainWindow));
			
			//controlWindow->InitializeControl(self->GetHwnd(self->m_controlWindow));
			});
			/*
		controlWindow->InitializeControl(GetHwnd(m_controlWindow));
		m_controlWindow.Activate();
		mainWindow->MainInitialize(GetHwnd(m_mainWindow));
		m_mainWindow.Activate();
		*/
	}
}