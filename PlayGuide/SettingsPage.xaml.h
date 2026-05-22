#pragma once
#include "SettingsPage.g.h"
#include "AppSettingsViewModel.h"
#include "Loc.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

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
        void HotkeyFlyout_Opening(IInspectable const& sender, IInspectable const&);
        void HotkeyCaptureBorder_KeyDown(IInspectable const&, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void HotkeyFlyout_Closed(IInspectable const&, IInspectable const&);
        void SaveCapture_Click(IInspectable const&, RoutedEventArgs const&);
        void CommitHotkey();
        void CancelCapture_Click(IInspectable const&, RoutedEventArgs const&);
        PlayGuide::HotkeyItemViewModel m_currentEditing{ nullptr };

        Button m_hotkeyButton{ nullptr };
        Flyout m_flyout{ nullptr };
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
