#pragma once
#include "AppSettingsViewModel.g.h"
#include "Appdata.h"
#include "AppDataService.h"

#include "HotkeyItemViewModel.h"
#include <winrt/Windows.Foundation.Collections.h>
#include "OptionItem.h"
#include "LocalizationHelper.h"
#include "ThemeService.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::PlayGuide::implementation
{
    struct AppSettingsViewModel :  AppSettingsViewModelT<AppSettingsViewModel>
    //winrt::implements<AppSettingsViewModel, IAppSettingsViewModel, winrt::Microsoft::UI::Xaml::Data::INotifyPropertyChanged>
    {
        AppSettingsViewModel()
        {
            m_hotkeys = single_threaded_observable_vector<winrt::PlayGuide::HotkeyItemViewModel>();

            m_pSettings = AppDataService::Get().AppSettingsPtr();

            auto hotkeys = AppDataService::Get().HotKeyMapCache();

            for (auto& [key, value] : g_hotkeyIconGlyphs)
            {
                auto it = hotkeys.find(key);
                if (it != hotkeys.end())
                {
                    auto item = winrt::make<HotkeyItemViewModel>(
                        LocalizationHelper::Get().String(winrt::hstring(key + L"_Name")),
                        LocalizationHelper::Get().String(winrt::hstring(key + L"_Description")),
                        it->second.GetString().c_str(),
                        value.c_str()
                    );

                    m_hotkeys.Append(item);
                }
            }

            //初始化选项列表
            m_languageList = single_threaded_observable_vector<PlayGuide::OptionItem>();
            m_languageList.Append(make<PlayGuide::implementation::OptionItem>(L"SystemDefault", 0));
            m_languageList.Append(make<PlayGuide::implementation::OptionItem>(L"Chinese", 1));
            m_languageList.Append(make<PlayGuide::implementation::OptionItem>(L"English", 2));

            m_themeList = single_threaded_observable_vector < PlayGuide::OptionItem>();
            m_themeList.Append(make<PlayGuide::implementation::OptionItem>(L"SystemDefault", (int)LocaleTheme::System));
            m_themeList.Append(make<PlayGuide::implementation::OptionItem>(L"Dark", (int)LocaleTheme::Dark));
            m_themeList.Append(make<PlayGuide::implementation::OptionItem>(L"Light", (int)LocaleTheme::Light));

            m_inputMethodList = single_threaded_observable_vector<PlayGuide::OptionItem>();
           
            m_inputMethodList.Append(make<PlayGuide::implementation::OptionItem>(L"Hook", (int)InputType::KeyboardHook));
            m_inputMethodList.Append(make<PlayGuide::implementation::OptionItem>(L"RawInput", (int)InputType::RawInput));

            m_selectedLanguage = m_languageList.GetAt(static_cast<int>(m_pSettings->language));
            m_selectedTheme = m_themeList.GetAt(static_cast<int>(m_pSettings->theme));
            m_selectedInputMethod = m_themeList.GetAt(static_cast<int>(m_pSettings->inputType));
        }
    private:
        winrt::event<
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    public:

        winrt::event_token PropertyChanged(
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_propertyChanged.add(handler);
        }

        void PropertyChanged(winrt::event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }

        void RaisePropertyChanged(winrt::hstring const& propertyName)
        {
            m_propertyChanged(*this,
                winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
        }

        template<typename T>
        bool SetProperty(T& field, T const& value, winrt::hstring const& name)
        {
            if (field == value)
                return false;

            field = value;
            RaisePropertyChanged(name);
            return true;
        }
        // ======================
        // 设置访问接口（推荐）
        // ======================

        int Theme()
        {
            return (int)m_pSettings->theme;
        }

        void Theme(int value)
        {
            m_pSettings->theme = (LocaleTheme)value;
            RaisePropertyChanged(L"Theme");
        }

        int Language()
        {
            return (int)m_pSettings->language;
        }

        void Language(int value)
        {
            m_pSettings->language = (LocaleLanguage) value;
            RaisePropertyChanged(L"Language");
        }

        bool AutoStart()
        {
            return m_pSettings->autoStart;
        }

        void AutoStart(bool v)
        {
            m_pSettings->autoStart = v;
            RaisePropertyChanged(L"AutoStart");
        }

        void InputType(int value) noexcept
        {
            m_pSettings->inputType = static_cast<::InputType>(value);
            RaisePropertyChanged(L"InputType");
        }

        int InputType() noexcept 
        {
            return (int)m_pSettings->inputType;
        }

        IObservableVector<winrt::PlayGuide::HotkeyItemViewModel> Hotkeys() noexcept
        {
            return m_hotkeys;
        }

        void Hotkeys(IObservableVector<winrt::PlayGuide::HotkeyItemViewModel>const& value) noexcept
        {
            m_hotkeys = value;
            RaisePropertyChanged(L"Hotkeys");
        }

        bool HotkeysEnabled() noexcept
        {
             return m_hotkeysEnabled;
        }

        void HotkeysEnabled(bool value) noexcept
        {
            m_hotkeysEnabled = value;
            SetProperty(m_hotkeysEnabled, value, L"HotkeysEnabled");
            //RaisePropertyChanged(L"HotkeysEnabled");
        }

        bool RunningAsAdmin() noexcept
        {
            return m_isRunningAsAdmin;
        }

        void RunningAsAdmin(bool value) noexcept
        {
            m_isRunningAsAdmin = value;
            SetProperty(m_isRunningAsAdmin, value, L"RunningAsAdmin");
        }
      
        bool IntelCpuUseECore() noexcept
        {
            return m_isIntelCpuUseECore;
        }

        void IntelCpuUseECore(bool value) noexcept
        {
            m_isIntelCpuUseECore = value;
            SetProperty(m_isIntelCpuUseECore, value, L"IntelCpuUseECore");
        }

        bool SystemTrayExecute() noexcept
        {
            return m_isSystemTrayExecute;
        }

        void SystemTrayExecute(bool value) noexcept
        {
            m_isSystemTrayExecute = value;
            SetProperty(m_isSystemTrayExecute, value, L"SystemTrayExecute");
            //RaisePropertyChanged(L"SystemTrayExecute");
        }

        IObservableVector<PlayGuide::OptionItem> LanguageList() noexcept
        {
            if (!m_languageList.Size())
            {
                LOG_DEBUG << "m_languageList is empty\n";
            }
            return m_languageList;
        }

        IObservableVector<PlayGuide::OptionItem> ThemeList() noexcept
        {
            return m_themeList;
        }

        IObservableVector<PlayGuide::OptionItem> InputMethodList() noexcept
        {
            return m_inputMethodList;
        }

        auto SelectedLanguage() noexcept { return m_selectedLanguage; }
        void SelectedLanguage(PlayGuide::OptionItem const& value) noexcept 
        { 
            if (m_selectedLanguage.Value() == value.Value())
                return;
            m_selectedLanguage = value;
            auto code = GetLanguageCode(value.Value());
            LocalizationHelper::Get().SetLanguage(code);
        }

        auto SelectedTheme() noexcept { return m_selectedTheme; }
        void SelectedTheme(PlayGuide::OptionItem const& value) noexcept 
        { 
            if (m_selectedTheme.Value() == value.Value())
                return;
            m_selectedTheme = value;
            ThemeService::SetTheme((LocaleTheme)value.Value());
            AppDataService::Get().SaveTheme((LocaleTheme)value.Value());
        }

        auto SelectedInputMethod() noexcept { return m_selectedInputMethod; }
        void SelectedInputMethod(PlayGuide::OptionItem const& value) noexcept { m_selectedInputMethod = value; }
    private:
        winrt::hstring GetLanguageCode(int32_t index) noexcept
        {
            static const std::unordered_map<int32_t, winrt::hstring> languageCodeMap =
            {
                { 0, L"" },          // System Default
                { 1, L"zh-Hans" },
                { 2, L"en" }
            };
            auto it = languageCodeMap.find(index);

            if (it != languageCodeMap.end())
            {
                return it->second;
            }

            return L"";
        }
    private:
        AppSettings* m_pSettings;
        IObservableVector<winrt::PlayGuide::HotkeyItemViewModel> m_hotkeys;

        bool m_hotkeysEnabled{ true };
        bool m_isRunningAsAdmin{ false };
        bool m_isIntelCpuUseECore{ false };
        bool m_isSystemTrayExecute{ true };

        IObservableVector<winrt::PlayGuide::OptionItem> m_languageList{ nullptr };
        IObservableVector<winrt::PlayGuide::OptionItem> m_themeList{ nullptr };
        IObservableVector<winrt::PlayGuide::OptionItem> m_inputMethodList{ nullptr };
        
        PlayGuide::OptionItem m_selectedLanguage{ nullptr };
        PlayGuide::OptionItem m_selectedTheme{ nullptr };
        PlayGuide::OptionItem m_selectedInputMethod{ nullptr };
};
}

namespace winrt::PlayGuide::factory_implementation
{
    struct AppSettingsViewModel : AppSettingsViewModelT<AppSettingsViewModel, implementation::AppSettingsViewModel>
    {
    };
}

