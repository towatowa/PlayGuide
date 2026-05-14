#pragma once
#include "HotkeyItemViewModel.g.h"

#include <winrt/Windows.Foundation.Collections.h>

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
        IObservableVector<winrt::hstring>m_splitKeys{ single_threaded_observable_vector<winrt::hstring>() };
    public:
        HotkeyItemViewModel() = default;
        HotkeyItemViewModel(
            winrt::hstring const& name,
            winrt::hstring const& description,
            winrt::hstring const& key,
            winrt::hstring const& icon)
            : m_name(name), m_description(description), m_key(key), m_icon(icon)
        { 
            auto parts = winrt::single_threaded_observable_vector<winrt::hstring>();
            std::wstring str{ m_key };
            size_t start = 0, end;
            while ((end = str.find(L"+", start)) != std::wstring::npos) {
                parts.Append(str.substr(start, end - start).c_str());
                start = end + 1;
            }
            parts.Append(str.substr(start).c_str());
            m_splitKeys = parts;
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

        Windows::Foundation::Collections::IObservableVector<winrt::hstring> SplitKeys() noexcept
        {
            return m_splitKeys;
        }

        void SplitKeys(IObservableVector<hstring> const&value) noexcept
        {
            m_splitKeys = value;
            RaisePropertyChanged(L"SplitKeys");
        }
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct HotkeyItemViewModel : HotkeyItemViewModelT<HotkeyItemViewModel, implementation::HotkeyItemViewModel>
    {
    };
}
