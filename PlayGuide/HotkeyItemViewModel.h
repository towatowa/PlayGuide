#pragma once
#include "HotkeyItemViewModel.g.h"

#include <winrt/Windows.Foundation.Collections.h>
#include "AppDataService.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::PlayGuide::implementation
{
    struct HotkeyItemViewModel : HotkeyItemViewModelT<HotkeyItemViewModel>
    {
    private:
        winrt::hstring m_name;
        winrt::hstring m_description;
        winrt::hstring m_key;
        winrt::hstring m_icon;
        winrt::hstring m_iniKey{L""};
        IObservableVector<winrt::hstring>m_splitKeys{ single_threaded_observable_vector<winrt::hstring>() };
        winrt::Windows::Foundation::Collections::IObservableVector<hstring> m_recordingKeys
        {
            winrt::single_threaded_observable_vector<hstring>()
        };
    public:
        HotkeyItemViewModel() = default;
        HotkeyItemViewModel(
            winrt::hstring const& name,
            winrt::hstring const& description,
            winrt::hstring const& key,
            winrt::hstring const& icon,
            winrt::hstring const& iniKeyName)
            : m_name(name), m_description(description), m_key(key), m_icon(icon), m_iniKey(iniKeyName)
        { 
            auto keys = SplitString(key);
            for (auto key : keys)
                m_splitKeys.Append(key);
        }

    private:
        winrt::event<
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        IObservableVector<hstring> SplitString(hstring const&key) noexcept;
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
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
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

        winrt::hstring Name()
        {
            return m_name;
        }

        void Name(winrt::hstring const& value)
        {
            SetProperty(m_name, value, L"Name");
        }

        winrt::hstring Description() noexcept
        {
            return m_description;
        }
        
        void Description(winrt::hstring const& value)
        {
            SetProperty(m_description, value, L"Description");
        }

        winrt::hstring Key()
        {
            return m_key;
        }

        void Key(winrt::hstring const& value)
        {
            SetProperty(m_key, value, L"Key");
        }

        winrt::hstring IconGlyph()
        {
            return m_icon;
        }

        void IconGlyph(winrt::hstring const& value)
        {
            SetProperty(m_icon, value, L"IconGlyph");
        }

        IObservableVector<winrt::hstring> SplitKeys() noexcept
        {
            return m_splitKeys;
        }

        void SplitKeys(IObservableVector<hstring> const&value) noexcept
        {
            m_splitKeys.Clear();
            for (auto key : value)
                m_splitKeys.Append(key);
            RaisePropertyChanged(L"SplitKeys");
        }

        IObservableVector<hstring> RecordingKeys() noexcept
        {
            return m_recordingKeys;
        }

        fire_and_forget SetHotkey(hstring const& key) noexcept
        {
            m_key = key;
            RaisePropertyChanged(L"Key");
            auto keys = SplitString(key);

            m_splitKeys.Clear();

            auto newVector = winrt::single_threaded_observable_vector<hstring>();
            for (auto const& k : keys)
            {
                newVector.Append(k);
            }
            m_splitKeys = newVector;
            RaisePropertyChanged(L"SplitKeys");
            
            AppDataService::Get().SaveSettingItem(L"Hotkey", m_iniKey.c_str(), key.c_str());
            co_return;
        }
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct HotkeyItemViewModel : HotkeyItemViewModelT<HotkeyItemViewModel, implementation::HotkeyItemViewModel>
    {
    };
}
