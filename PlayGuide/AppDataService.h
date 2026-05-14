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
	void SaveAppSettings() const;
	void SaveAppSettings(const AppSettings* settings) const;
	// =========================
	// runtime cache
	// =========================
	const MainWindowData& GetMainDataCache() const { return m_mainData; }
	const ControlWindowData& GetControlDataCache() const { return m_controlData; }
	const HotKeyMap& GetHotKeyMapCache() const { return m_hotkeyMap; };
	const std::unordered_map<Key, UINT>& GetHotKeyCache() const { return m_hotkey; };
	AppSettings* GetAppSettings() noexcept { return &m_settings; }

	InputType GetInputType() const { return m_settings.inputType; }

	bool GetHotkeyEnableState() noexcept { return m_hotkeysEnabled; }

	bool ToggleHotkeysEnabled() noexcept
	{
		m_hotkeysEnabled = !m_hotkeysEnabled;
		return m_hotkeysEnabled;
	}
private:
	AppDataService() = default;
	~AppDataService() = default;

	AppDataService(const AppDataService&) = delete;
	AppDataService& operator=(const AppDataService&) = delete;

private:

	MainWindowData m_mainData{};
	ControlWindowData m_controlData{};
	HotKeyMap m_hotkeyMap{};
	std::unordered_map<Key, UINT>m_hotkey;//service runtime cache
	std::unique_ptr<IniHelper> m_ini;
	AppSettings m_settings{};
	bool m_hotkeysEnabled{ true };
};