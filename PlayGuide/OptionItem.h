#pragma once
#include "OptionItem.g.h"

namespace winrt::PlayGuide::implementation
{
    struct OptionItem : OptionItemT<OptionItem>
    {
        OptionItem() = default;
        explicit OptionItem(winrt::hstring const& text, int value) : m_text(text), m_value(value) {};
        explicit OptionItem(winrt::hstring const& text, int value, winrt::hstring const& description, winrt::hstring const& icon) :
            m_text(text), m_value(value), m_description(description), m_icon(icon) {
        };

        winrt::hstring Text() noexcept
        {
            return m_text;
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
