#pragma once
#include "AboutWindow.g.h"
#include <winrt/Microsoft.UI.Windowing.h>
#include "AppSettingsViewModel.h"
#include "Appdata.h"
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include "event.h"
#include "Global.h"
#include "Loc.h"

using namespace Microsoft::UI::Composition::SystemBackdrops;
using namespace Microsoft::UI::Xaml;

namespace winrt::PlayGuide::implementation
{
    struct AboutWindow : AboutWindowT<AboutWindow>
    {
        AboutWindow()
        {
            this->AppWindow().Resize({ 840, 700 });

            this->ExtendsContentIntoTitleBar(true);
            DispatcherQueue().TryEnqueue([this]() {
                InitializeInfo();
                LocaleTheme theme = static_cast<LocaleTheme>(AppSettingsViewModel::Instance().Theme());
                ElementTheme xamlTheme = ElementTheme::Default;

                switch (theme)
                {
                case LocaleTheme::System:
                    xamlTheme = ElementTheme::Default;
                    break;

                case LocaleTheme::Light:
                    xamlTheme = ElementTheme::Light;
                    break;

                case LocaleTheme::Dark:
                    xamlTheme = ElementTheme::Dark;
                    break;
                }
                this->RootGrid().RequestedTheme(xamlTheme);
                });
            m_languageChangedEventRevoker = g_languageChanged(auto_revoke, [this]() {
                Loc::RefreshTree(this->Content());
                });
        }

        void InitializeInfo();

        void OnOpenGithub(
            Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);

        Windows::Foundation::IAsyncAction OnCheckUpdate(
            Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void OnCopyInfo(
            Windows::Foundation::IInspectable const& sender,
            Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnOpenLicense(IInspectable const&, RoutedEventArgs const&);
        Event<>::EventRevoker m_languageChangedEventRevoker;
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct AboutWindow : AboutWindowT<AboutWindow, implementation::AboutWindow>
    {
    };
}
