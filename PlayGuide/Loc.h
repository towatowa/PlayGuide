#pragma once
#include "loc.g.h"
#include <winrt/Microsoft.UI.Xaml.h>

namespace winrt::PlayGuide::implementation
{
    struct Loc : LocT<Loc>
    {
        static winrt::Microsoft::UI::Xaml::DependencyProperty TextProperty();

        static void SetText(
            winrt::Microsoft::UI::Xaml::DependencyObject const& obj,
            winrt::hstring const& value);

        static winrt::hstring GetText(
            winrt::Microsoft::UI::Xaml::DependencyObject const& obj);

        static void RefreshTree(
            winrt::Microsoft::UI::Xaml::DependencyObject const& root);
    private:

        static void OnTextChanged(
            winrt::Microsoft::UI::Xaml::DependencyObject const& d,
            winrt::Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e);
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct Loc : LocT<Loc, implementation::Loc>
    {
    };
}