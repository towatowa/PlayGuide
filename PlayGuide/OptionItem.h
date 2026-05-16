#pragma once
#include "OptionItem.g.h"
#include "LocalizationHelper.h"

namespace winrt::PlayGuide::implementation
{
    struct OptionItem : OptionItemT<OptionItem>
    {
        OptionItem() = default;
        explicit OptionItem(hstring const& key, int value) :
            m_key(key), m_value(value) {};
        explicit OptionItem(hstring const& key, int value, hstring const& description, winrt::hstring const& icon) :
            m_key(key), m_value(value), m_description(description), m_icon(icon) {
        };

        winrt::hstring Text() noexcept
        {
            return m_text = LocalizationHelper::Get().String(m_key);
        }

        void Text(winrt::hstring const& value) noexcept
        {
            m_text = value;
        }

        int Value() noexcept
        {
            return m_value;
        }

        void Value(int const &value) noexcept
        {
            m_value = value;
        }

        winrt::hstring Description() noexcept
        {
            return m_description;
        }
        
        void Description(winrt::hstring const& value) noexcept
        {
            m_description = value;
        }

        winrt::hstring Icon() noexcept
        {
            return m_icon;
        }

        void Icon(winrt::hstring const& value) noexcept
        {
            m_icon = value;
        }

    private:
        winrt::hstring m_key{ L"" };
        winrt::hstring m_text{ L"" };
        int m_value{ 0 };
        winrt::hstring m_description{ L"" };
        winrt::hstring m_icon;
    };

}

namespace winrt::PlayGuide::factory_implementation
{
    struct OptionItem : OptionItemT<OptionItem, implementation::OptionItem>
    {
    };
}
