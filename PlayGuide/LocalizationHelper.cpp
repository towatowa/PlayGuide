#include "pch.h"
#include "LocalizationHelper.h"

using namespace winrt::Microsoft::Windows::ApplicationModel::Resources;

winrt::hstring LocalizationHelper::ResolveLanguage(winrt::hstring const& input)
{
    // 1. 精确匹配优先
    for (auto& _lang : g_supportLanguageList)
    {
        if (input == _lang)
            return _lang;
    }

    // 2. 前缀匹配（zh-Hans-CN -> zh-Hans）
    for(int i = 0; i < g_supportLanguageList.size(); i++)
    {
        if (input.starts_with(g_supportLanguageList[i]))
            return g_supportLanguageList[i];
    }

    // 3. zh 特殊兜底
    if (input.starts_with(L"zh"))
    {
        return L"zh-Hans";
    }

    // 4. 最终 fallback
    return L"en";
}
