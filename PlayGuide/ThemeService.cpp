#include "pch.h"
#include "ThemeService.h"
#include "AppDataService.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

ElementTheme ThemeService::m_theme = ElementTheme::Default;

std::vector<Window> ThemeService::m_windows;

void ThemeService::RegisterWindow(Window const& window)
{
    m_windows.push_back(window);

    ApplyToWindow(window);
}

void ThemeService::ApplyToWindow(Window const& window)
{
    if (auto root = window.Content().try_as<FrameworkElement>())
    {
        root.RequestedTheme(m_theme);
    }
}

void ThemeService::SetTheme(LocaleTheme theme)
{
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
    m_theme = xamlTheme;
    for (auto const& window : m_windows)
    {
        ApplyToWindow(window);
    }
}
