#include <winrt/Microsoft.UI.Xaml.h>
#include <Appdata.h>

class ThemeService
{
public:
    static void SetTheme(LocaleTheme theme);
    static void RegisterWindow(winrt::Microsoft::UI::Xaml::Window const& window);

private:
    static winrt::Microsoft::UI::Xaml::ElementTheme m_theme;
    static std::vector<winrt::weak_ref<winrt::Microsoft::UI::Xaml::Window>> m_windows;

    static void ApplyToWindow(
        winrt::weak_ref<winrt::Microsoft::UI::Xaml::Window>& window);
};