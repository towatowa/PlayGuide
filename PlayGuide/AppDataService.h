#pragma once
#include "Appdata.h"
#include "IniHelper.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>

class AppDataService
{
public:
    static AppDataService& Get()
    {
        static AppDataService ins;
        return ins;
    }

    // =========================
    // 初始化
    // =========================
    void Initialize(const std::wstring& path);
    void CreateDefaultConfig(const std::wstring& path);
    // =========================
    // Load
    // =========================
    MainWindowData LoadMainData() const;
    ControlWindowData LoadControlData() const;
    HotKeyMap LoadHotkeys() const;
    std::vector<std::wstring> LoadUrls() const;

    // =========================
    // Save
    // =========================
    void SaveMainData(const MainWindowData& data);
    void SaveControlData(const ControlWindowData& data);
    void SaveHotkeys(const HotKeyMap& hotkeys);
    void SaveUrls(const std::vector<std::wstring>& data);
    // =========================
    // runtime cache
    // =========================
    const MainWindowData& GetMainDataCache() const { return m_mainData; }
    const ControlWindowData& GetControlDataCache() const { return m_controlData; }
    const HotKeyMap& GetHotKeyMapCache() const { return m_hotkeyMap; };
    const std::unordered_map<UINT, USHORT>& GetHotKeyCache() const { return m_hotkey; };
private:
    AppDataService() = default;
    ~AppDataService() = default;

    AppDataService(const AppDataService&) = delete;
    AppDataService& operator=(const AppDataService&) = delete;

private:
   
    MainWindowData m_mainData{};
    ControlWindowData m_controlData{};
    HotKeyMap m_hotkeyMap{};
    std::unordered_map<UINT, USHORT>m_hotkey;
    std::unique_ptr<IniHelper> m_ini;
};