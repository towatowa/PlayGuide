#pragma once
#include "Logger.h"
#include <mutex>

#include <winrt/Microsoft.Windows.ApplicationModel.Resources.h>
#include <winrt/Microsoft.Windows.Globalization.h>
#include <winrt/Microsoft.Windows.AppLifecycle.h>

using namespace winrt::Microsoft::Windows::ApplicationModel::Resources;

inline std::vector<winrt::hstring>g_supportLanguageList = {
    L"en",
    L"zh-Hans"
};

class LocalizationHelper
{
public:
    static LocalizationHelper& Get()
    {
        static LocalizationHelper instance;
        return instance;
    }

    LocalizationHelper(const LocalizationHelper&) = delete;
    LocalizationHelper& operator=(const LocalizationHelper&) = delete;

public:
    void Initialize()
    {
        try
        {
            std::lock_guard lock(m_mutex);

            // ❗延迟创建，避免 main/Loaded 崩溃
            if (!m_resourceManager)
            {
                m_resourceManager = ResourceManager();
                m_mainMap = m_resourceManager.MainResourceMap();
                m_context = m_resourceManager.CreateResourceContext();
            }

            LOG_INFO << "LocalizationHelper initialized.";
        }
        catch (winrt::hresult_error const& e)
        {
            LOG_ERROR << "Init failed: " << winrt::to_string(e.message());
        }
    }

    winrt::hstring String(winrt::hstring const& key,
        winrt::hstring const& mapName = L"Resources")
    {
        try
        {
            EnsureInit();
            std::lock_guard lock(m_mutex);
            auto path = mapName + L"/" + key;
            auto value = m_mainMap.TryGetValue(path, m_context);
            if (value)
                return value.ValueAsString();
        }
        catch (winrt::hresult_error const& e)
        {
            LOG_WARN << "Missing key: " << winrt::to_string(key)
                << " err: " << winrt::to_string(e.message());
        }

        return L"[" + key + L"]";
    }

    void SetLanguage(winrt::hstring const& languageCode)
    {
        try
        {
            std::lock_guard lock(m_mutex);
            auto language = ResolveLanguage(languageCode);
            winrt::Microsoft::Windows::Globalization::ApplicationLanguages::
                PrimaryLanguageOverride(language);

            if (m_resourceManager)
            {
                m_context = m_resourceManager.CreateResourceContext();
            }

            LOG_INFO << "Language changed: " << winrt::to_string(languageCode);
        }
        catch (winrt::hresult_error const& e)
        {
            LOG_ERROR << "SetLanguage failed: " << winrt::to_string(e.message());
        }
    }

    void RestartApp()
    {
        winrt::Microsoft::Windows::AppLifecycle::AppInstance::Restart(L"");
    }
    winrt::hstring ResolveLanguage(winrt::hstring const& input);

private:
    LocalizationHelper() = default;

    void EnsureInit()
    {
        // ❗lazy init（避免 main / ViewModel 构造阶段崩溃）
        if (!m_resourceManager)
            Initialize();
    }

    

private:
    std::mutex m_mutex;

    ResourceManager m_resourceManager{ nullptr };
    ResourceMap      m_mainMap{ nullptr };
    ResourceContext   m_context{ nullptr };
};

