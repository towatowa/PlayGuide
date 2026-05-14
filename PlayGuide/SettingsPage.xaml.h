#pragma once
#include "SettingsPage.g.h"
#include "AppSettingsViewModel.h"

namespace winrt::PlayGuide::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
        SettingsPage()
        {
            m_viewModel = winrt::make<winrt::PlayGuide::implementation::AppSettingsViewModel>();
            /*
            DispatcherQueue().TryEnqueue([this](){
                HotkeyList().ItemsSource(m_viewModel.Hotkeys());
            });
            */
            this->Loaded([this](auto&&, auto&&) {
                HotkeyList().ItemsSource(m_viewModel.Hotkeys());
            });
        }

        auto ViewModel() noexcept
        {
            return m_viewModel;
        }

        winrt::PlayGuide::AppSettingsViewModel m_viewModel;
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
