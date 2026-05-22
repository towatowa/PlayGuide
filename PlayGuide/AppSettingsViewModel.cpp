#include "pch.h"
#include "AppSettingsViewModel.h"
#include "HotkeyItemViewModel.h" 
#include "AppSettingsViewModel.g.cpp" // 必须包含这个生成的文件！
#include "LocalizationHelper.h"

namespace winrt::PlayGuide::implementation
{
    void AppSettingsViewModel::UpdateHotkeysList() noexcept
    {
        //auto hotkeysList = single_threaded_observable_vector<winrt::PlayGuide::HotkeyItemViewModel>();
        m_hotkeys.Clear();
        auto hotkeys = AppDataService::Get().HotKeyMapCache();

        for (auto& [key, value] : g_hotkeyIconGlyphs)
        {
            auto it = hotkeys.find(key);
            if (it != hotkeys.end())
            {
                auto item = winrt::make<HotkeyItemViewModel>(
                    LocalizationHelper::Get().String(winrt::hstring(key + L"_Name")),
                    LocalizationHelper::Get().String(winrt::hstring(key + L"_Description")),
                    it->second.GetString().c_str(),
                    value.c_str(),
                    key.c_str()
                );

                m_hotkeys.Append(item);
            }
        }
        //m_hotkeys = hotkeysList;
        RaisePropertyChanged(L"Hotkeys");
    }
}
