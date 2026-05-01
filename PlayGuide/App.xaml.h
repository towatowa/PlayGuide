#pragma once

#include "App.xaml.g.h"
#include "MainWindow.xaml.h"
#include "ControlWindow.xaml.h"
#include "Event.h"

namespace winrt::PlayGuide::implementation
{
    struct App : AppT<App>
    {
        App();

        HWND GetHwnd(winrt::Microsoft::UI::Xaml::Window const& window);

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        Event<bool>::EventRevoker closeControlWindowEvent;
        
    private:
        winrt::Microsoft::UI::Xaml::Window m_mainWindow{ nullptr };
        winrt::Microsoft::UI::Xaml::Window m_controlWindow{ nullptr };
    };
}
