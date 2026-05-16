#include "pch.h"
#include "LocalizationHelper.h"
#include <winrt/Microsoft.Windows.Globalization.h>

using namespace winrt::Microsoft::Windows::ApplicationModel::Resources;

winrt::hstring LocalizationHelper::ResolveLanguage(winrt::hstring const& input)
{
    winrt::hstring language = input;

    // 0. 空字符串 -> 系统默认语言
    if (language.empty())
    {
        auto languages =
            winrt::Microsoft::Windows::Globalization::ApplicationLanguages::Languages();

        if (languages.Size() > 0)
        {
            language = languages.GetAt(0);
        }
    }

    // 1. 精确匹配
    for (auto const& lang : g_supportLanguageList)
    {
        if (language == lang)
            return lang;
    }

    // 2. 前缀匹配（zh-Hans-CN -> zh-Hans）
    for (auto const& lang : g_supportLanguageList)
    {
        if (language.starts_with(lang))
            return lang;
    }

    // 3. zh fallback
    if (language.starts_with(L"zh"))
    {
        return L"zh-Hans";
    }

    // 4. default fallback
    return L"en";
}
