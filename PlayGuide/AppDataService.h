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
	AppSettings LoadSettings() const;
	// =========================
	// Save
	// =========================
	void SaveMainData(const MainWindowData& data);
	void SaveControlData(const ControlWindowData& data);
	void SaveHotkeys(const HotKeyMap& hotkeys);
	void SaveUrls(const std::vector<std::wstring>& data);
	void SaveAppSettings() const;
	void SaveAppSettings(const AppSettings* settings) const;

	template<class T>
	void SaveSettingItem(const std::wstring& section, const std::wstring& key, const T& value);
	// =========================
	// runtime cache
	// =========================
	const MainWindowData& MainDataCache() const { return m_mainData; }
	const ControlWindowData& ControlDataCache() const { return m_controlData; }
	const HotKeyMap& HotKeyMapCache() const { return m_hotkeyMap; };
	const std::unordered_map<Key, UINT>& HotKeyToMsgMapping() const { return m_hotkey; };
	AppSettings* AppSettingsPtr() noexcept { return &m_settings; }

	InputType InputType() const { return m_settings.inputType; }

	//global hotkey Switch
	bool HotkeyEnableState() noexcept { return m_hotkeysEnabled; }
	bool ToggleHotkeysEnabled() noexcept;
	//当前主题
	LocaleTheme Theme() noexcept { return m_settings.theme; }
	void SaveTheme(LocaleTheme theme);
	//system tray
	bool SystemTray() noexcept { return m_settings.systemTrayExecute; }
	bool ToggleSystemTray() noexcept;
	bool RunAsAdmin() noexcept { return m_settings.adminRunning; }
	bool ToggleRunAsAdmin() noexcept;

	AppDataService() = default;
	~AppDataService() = default;
	//自启动
	bool AutoStart() noexcept { return m_settings.autoStart; }
	bool ToggleAutoStart() noexcept;
	//cpu亲和性优化
	bool IntelCpuUseEcore() noexcept { return m_settings.intelCpuUseECore; }
	bool ToggleIntelCpuUseEcore() noexcept;

	//主窗口状态
	WindowState MainWindowState() noexcept { return m_mainData.windowState; }
	void SetMainWindowState(WindowState state) noexcept;
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

template<class>
inline constexpr bool always_false = false;

template<class T>
void AppDataService::SaveSettingItem(const std::wstring &section, const std::wstring &key, const T &value)
{
	if constexpr (std::is_same_v<T, int>)
	{
		m_ini->WriteInt(section, key, value);
	}
	else if constexpr (std::is_same_v<T, bool>)
	{
		m_ini->WriteInt(section, key, value ? 1 : 0);
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		m_ini->WriteString(section, key, std::to_wstring(value));
	}
	else if constexpr (std::is_same_v<T, std::wstring>)
	{
		m_ini->WriteString(section, key, value);
	}
	else if constexpr (std::is_same_v<T, std::wstring_view>)
	{
		m_ini->WriteString(section, key, std::wstring(value));
	}
	else
	{
		static_assert(always_false<T>, "Unsupported type for SaveSettingItem");
	}
}