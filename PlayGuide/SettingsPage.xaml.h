#pragma once
#include "SettingsPage.g.h"
#include "AppSettingsViewModel.h"
#include "Loc.h"

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
            auto weak_this = this->get_weak();
            this->Loaded([weak_this](auto&&, auto&&) {
                if (auto self = weak_this.get()) 
                {
                    self->LanguageComboBox().SelectionChanged([self](auto&&, auto&&) {
                        Loc::RefreshTree(*self);
                        });
                    self->HotkeyList().ItemsSource(self->m_viewModel.Hotkeys());
                }
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
