#pragma once
#include "AppSettingsViewModel.g.h"
#include "Appdata.h"
#include "AppDataService.h"

#include "HotkeyItemViewModel.h"
#include <winrt/Windows.Foundation.Collections.h>
#include "OptionItem.h"
#include "LocalizationHelper.h"
#include "ThemeService.h"
#include "TrayIconService.h"
#include "Win32Helper.h"
#include <unordered_set>
#include "Event.h"
#include "Global.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::PlayGuide::implementation
{
    struct AppSettingsViewModel :  AppSettingsViewModelT<AppSettingsViewModel>
    {
        inline static winrt::weak_ref<PlayGuide::AppSettingsViewModel> s_instance{ nullptr };
        static PlayGuide::AppSettingsViewModel Instance()
        {
            if (auto inst = s_instance.get())
            {
                return inst;
            }

            auto newInstance = winrt::make<PlayGuide::implementation::AppSettingsViewModel>();
            s_instance = newInstance;
            return newInstance;
        }

        /*
        // 禁止拷贝和赋值（单例必须）
        AppSettingsViewModel(AppSettingsViewModel const&) = delete;
        AppSettingsViewModel& operator=(AppSettingsViewModel const&) = delete;
        */
        AppSettingsViewModel()
        {
            m_hotkeys = single_threaded_observable_vector<winrt::PlayGuide::HotkeyItemViewModel>();

            m_pSettings = AppDataService::Get().AppSettingsPtr();

            UpdateHotkeysList();

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
            m_selectedInputMethod = m_inputMethodList.GetAt(static_cast<int>(m_pSettings->inputType));
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
            m_pSettings->language = (LocaleLanguage)value;
            RaisePropertyChanged(L"Language");
        }

        bool AutoStart()
        {
            return m_pSettings->autoStart;
        }

        void AutoStart(bool value)
        {
            if (m_pSettings->autoStart == value)
                return;
            if (AppDataService::Get().ToggleAutoStart())
                Win32Helper::SetAutoStart(true);
            else Win32Helper::SetAutoStart(false);
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

        auto Hotkeys() noexcept
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
             return AppDataService::Get().HotkeyEnableState();
        }

        void HotkeysEnabled(bool value) noexcept
        {
            auto state = AppDataService::Get().HotkeyEnableState();
            if (state == value)
                return;
            AppDataService::Get().ToggleHotkeysEnabled();
        }

        bool RunningAsAdmin() noexcept
        {
            return m_pSettings->adminRunning;
        }

        void RunningAsAdmin(bool value) noexcept
        {
            if (m_pSettings->adminRunning == value)
                return;
            if (AppDataService::Get().ToggleRunAsAdmin())
                m_restartReason.insert(L"RunAsAdmin");
            else m_restartReason.erase(L"RunAsAdmin");

            RaisePropertyChanged(L"HasRestartReason");
        }
      
        bool IntelCpuUseECore() noexcept
        {
            return m_pSettings->intelCpuUseECore;
        }

        void IntelCpuUseECore(bool value) noexcept
        {
            if (m_pSettings->intelCpuUseECore == value)
                return;
            if (AppDataService::Get().ToggleIntelCpuUseEcore()) 
            {
                //m_restartReason.insert(L"IntelCpuUseECore");
                if (Win32Helper::SetThreadToEfficientCores())
                    LOG_INFO << "SetThreadToEfficientCores successfull.\n";
                //Win32Helper::TestEfficientThread();
            }
            else
            {
                Win32Helper::ClearCpuAffinity();
            }
        }

        bool SystemTrayExecute() noexcept
        {
            return m_pSettings->systemTrayExecute;
        }

        void SystemTrayExecute(bool value) noexcept
        {
            if (m_pSettings->systemTrayExecute != value)
            {
                if (AppDataService::Get().ToggleSystemTray())
                    TrayIconService::Get().Show();
                else TrayIconService::Get().Hide();
            }
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
            if (LocalizationHelper::Get().SetLanguage((LocaleLanguage)value.Value())) //语言改变
            {
                UpdateHotkeysList();//更新列表
                RaisePropertyChanged(L"SeletedTheme");
                RaisePropertyChanged(L"SelectedInputMethod");
                RaisePropertyChanged(L"TrayOnText");
                RaisePropertyChanged(L"TrayOffText");
                g_languageChanged.Invoke();
            }
            AppDataService::Get().SaveLanguage((LocaleLanguage)value.Value());
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
        void SelectedInputMethod(PlayGuide::OptionItem const& value) noexcept 
        { 
            if (m_selectedInputMethod.Value() == value.Value())
                return;
            m_selectedInputMethod = value;
            AppDataService::Get().SaveInputMethod((::InputType)value.Value());
        }

        Microsoft::UI::Xaml::Visibility HasRestartReason(hstring const& key) noexcept { return m_restartReason.contains(key) ? Microsoft::UI::Xaml::Visibility::Visible : Microsoft::UI::Xaml::Visibility::Collapsed; }

        bool IsIntelCpu() noexcept { return Win32Helper::IsIntelHybridCPU(); }

        PlayGuide::HotkeyItemViewModel CurrentEditingHotkey() noexcept { return m_currentEditing; }

        hstring TrayOnText() noexcept { return LocalizationHelper::Get().String(L"On"); }
        hstring TrayOffText() noexcept { return LocalizationHelper::Get().String(L"Off"); }

        void UpdateHotkey(hstring const& key, hstring const& value) noexcept;

    private:
        AppSettings* m_pSettings;
        IObservableVector<winrt::PlayGuide::HotkeyItemViewModel> m_hotkeys;

        IObservableVector<winrt::PlayGuide::OptionItem> m_languageList{ nullptr };
        IObservableVector<winrt::PlayGuide::OptionItem> m_themeList{ nullptr };
        IObservableVector<winrt::PlayGuide::OptionItem> m_inputMethodList{ nullptr };
        
        PlayGuide::OptionItem m_selectedLanguage{ nullptr };
        PlayGuide::OptionItem m_selectedTheme{ nullptr };
        PlayGuide::OptionItem m_selectedInputMethod{ nullptr };

        std::unordered_set<hstring>m_restartReason;

        PlayGuide::HotkeyItemViewModel m_currentEditing{ nullptr };
        void UpdateHotkeysList() noexcept;
};
}

namespace winrt::PlayGuide::factory_implementation
{
    struct AppSettingsViewModel : AppSettingsViewModelT<AppSettingsViewModel, implementation::AppSettingsViewModel>
    {
    };
}

