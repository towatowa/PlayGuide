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
                    //auto vm = AppSettingsViewModel::Instance().try_as<PlayGuide::AppSettingsViewModel>();
                    //self->HotkeyList().ItemsSource(vm.Hotkeys());
                }
            });
           
        }

        auto ViewModel() noexcept
        {
            return AppSettingsViewModel::Instance().try_as<PlayGuide::AppSettingsViewModel>();
        }

        void HotkeyFlyout_Opening(IInspectable const& sender, IInspectable const&);
        void HotkeyCaptureBorder_KeyDown(IInspectable const&, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void HotkeyFlyout_Closed(IInspectable const&, IInspectable const&);
        void SaveCapture_Click(IInspectable const&, RoutedEventArgs const&);
        fire_and_forget CommitHotkey();
        void CancelCapture_Click(IInspectable const&, RoutedEventArgs const&);
        void OnApplyHomepageClicked(IInspectable const&, RoutedEventArgs const&);
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
