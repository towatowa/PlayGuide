#include "AppDataService.h"
#include "KeyMapping.h"
#include "PipeService.h"

void AppDataService::Initialize(const std::wstring& path)
{
	m_ini = std::make_unique<IniHelper>(path);

	CreateDefaultConfig(path);

	// 预加载
	m_mainData = LoadMainData();
	m_controlData = LoadControlData();
	m_hotkeyMap = LoadHotkeys();
	m_settings = LoadSettings();
	int idx = 1;
	for (auto& key : g_keys)
	{
		m_hotkey[m_hotkeyMap[key]] = WM_USER + idx;
		idx++;
	}
}

void AppDataService::SaveMainData(const MainWindowData& data)
{
	m_ini->WriteString(L"MainWindow", L"x",
		std::to_wstring(data.x));

	m_ini->WriteString(L"MainWindow", L"y",
		std::to_wstring(data.y));

	m_ini->WriteString(L"MainWindow", L"width",
		std::to_wstring(data.width));

	m_ini->WriteString(L"MainWindow", L"height",
		std::to_wstring(data.height));

	m_ini->WriteString(L"MainWindow", L"alpha",
		std::to_wstring(data.alpha));

	m_ini->WriteInt(L"MainWindow", L"windowState",
	   (int)data.windowState);
	//m_ini->WriteString(L"Web", L"url", data.url);
	//SaveUrls(data.urls);
}

void AppDataService::SaveHotkeys(const HotKeyMap& hotkeys)
{
	for (auto& [key, value] : hotkeys)
	{
		m_ini->WriteString(
			L"Hotkey",
			key,
			value.GetString()
		);
	}
}

void AppDataService::SaveUrls(const std::vector<TabInfo>& data)
{
	m_ini->WriteInt(L"Urls", L"urlCount", data.size());
	std::wstring key;
	for (int i = 0; i < data.size(); ++i)
	{
		key = L"url" + std::to_wstring(i);
		m_ini->WriteString(L"Urls", key, data[i].url);
		key = L"title" + std::to_wstring(i);
        m_ini->WriteString(L"Urls", key, data[i].title);
	}
}

void AppDataService::SaveAppSettings(const AppSettings* settings) const
{
	m_ini->WriteInt(L"AppSettings", L"Theme", (int)settings->theme);
	m_ini->WriteInt(L"AppSettings", L"Language", (int)settings->language);
	m_ini->WriteInt(L"AppSettings", L"AutoStart", settings->autoStart ? 1 : 0);
	m_ini->WriteInt(L"AppSettings", L"SystemTrayExecute", settings->systemTrayExecute ? 1 : 0);
	m_ini->WriteInt(L"AppSettings", L"AdminRunning", settings->adminRunning ? 1 : 0);
	m_ini->WriteInt(L"AppSettings", L"IntelCpuUseECore", settings->intelCpuUseECore ? 1 : 0);
	m_ini->WriteInt(L"AppSettings", L"InputType", (int)settings->inputType);
	m_ini->WriteString(L"AppSettings", L"HomePage", settings->homePage);
	m_ini->WriteInt(L"AppSettings", L"KeyboardOff", settings->keyboardOff ? 1 : 0);
	m_ini->WriteInt(L"AppSettings", L"EnableWindowSnapping", settings->enableWindowSnapping ? 1 : 0);
}

void AppDataService::SaveHotkey(std::wstring_view id, std::wstring_view key)
{
	auto newKey = Key(key);
	auto oldKey = m_hotkeyMap[id.data()];
	m_hotkeyMap[id.data()] = newKey;
	auto msg = m_hotkey[oldKey];
	m_hotkey.erase(oldKey);
	m_hotkey[newKey] = msg;
	PipeService::Get().SendHotkeyEdit(msg, newKey);
	m_ini->WriteString(L"Hotkey", id.data(), key.data());
}

HotKeyMap AppDataService::LoadHotkeys() const
{
	HotKeyMap map;

	for (auto& k : g_keys)
	{
		auto val = m_ini->ReadString(L"Hotkey", k, L"");
		map[k] = Key(val);
	}

	return map;
}

std::vector<TabInfo> AppDataService::LoadUrls() const
{
	// vector webUrls
	int count = m_ini->ReadInt(L"Urls", L"urlCount", 1);
	std::vector<TabInfo>urls;
	std::wstring key;
	for (int i = 0; i < count; i++)
	{
		key = L"url" + std::to_wstring(i);

		auto url = m_ini->ReadString(L"Urls", key, L"");
		key = L"title" + std::to_wstring(i);
		auto title = m_ini->ReadString(L"Urls", key, L"");
		if (url[0] && title[0])
			urls.emplace_back(TabInfo{ 0, title, url});
	}

	return urls;
}

AppSettings AppDataService::LoadSettings() const
{
	AppSettings settings;

	settings.language =
		static_cast<LocaleLanguage>(
			m_ini->ReadInt(L"AppSettings", L"Language", 0));

	settings.theme =
		static_cast<LocaleTheme>(
			m_ini->ReadInt(L"AppSettings", L"Theme", 0));
	
	settings.autoStart =
		m_ini->ReadInt(L"AppSettings", L"AutoStart", 0) != 0;

	settings.systemTrayExecute =
		m_ini->ReadInt(L"AppSettings", L"SystemTrayExecute", 0) != 0;

	settings.adminRunning =
		m_ini->ReadInt(L"AppSettings", L"AdminRunning", 0) != 0;

	settings.intelCpuUseECore =
		m_ini->ReadInt(L"AppSettings", L"IntelCpuUseECore", 0) != 0;

	settings.inputType =
		static_cast<::InputType>(
			m_ini->ReadInt(L"AppSettings", L"InputType", 0));
	settings.homePage = m_ini->ReadString(L"AppSettings", L"HomePage", L"https://google.com");
	settings.keyboardOff = static_cast<bool>(m_ini->ReadInt(L"AppSettings", L"KeyboardOff", 0));
	settings.enableWindowSnapping = static_cast<bool>(m_ini->ReadInt(L"AppSettings", L"EnableWindowSnapping", 1));

	return settings;
}

MainWindowData AppDataService::LoadMainData() const
{
	MainWindowData data;

	data.x = m_ini->ReadInt(L"MainWindow", L"x", 100);
	data.y = m_ini->ReadInt(L"MainWindow", L"y", 100);
	data.width = m_ini->ReadInt(L"MainWindow", L"width", 1280);
	data.height = m_ini->ReadInt(L"MainWindow", L"height", 720);
	data.alpha = m_ini->ReadInt(L"MainWindow", L"alpha", 255);
	data.windowState = (WindowState)m_ini->ReadInt(L"MainWindow", L"windowState", (int)WindowState::Normal);

	//data.url = m_ini->ReadString(L"Web", L"url", g_defaultWebUrl);

	return data;
}

void AppDataService::SaveControlData(const ControlWindowData& data)
{
	m_ini->WriteString(
		L"ControlWindow",
		L"x",
		std::to_wstring(data.x)
	);

	m_ini->WriteString(
		L"ControlWindow",
		L"y",
		std::to_wstring(data.y)
	);

	m_ini->WriteString(
		L"ControlWindow",
		L"width",
		std::to_wstring(data.width)
	);

	m_ini->WriteString(
		L"ControlWindow",
		L"height",
		std::to_wstring(data.height)
	);

	m_ini->WriteString(
		L"ControlWindow",
		L"alpha",
		std::to_wstring(data.alpha)
	);
	m_ini->RemoveSection(L"Urls");
	m_ini->WriteInt(L"Urls", L"selectedItem", (int)data.selectedItem);
	SaveUrls(data.urls);
}

ControlWindowData AppDataService::LoadControlData() const
{
	ControlWindowData data;

	data.x = m_ini->ReadInt(L"ControlWindow", L"x", 100);
	data.y = m_ini->ReadInt(L"ControlWindow", L"y", 100);
	data.width = m_ini->ReadInt(L"ControlWindow", L"width", 1280);
	data.height = m_ini->ReadInt(L"ControlWindow", L"height", 720);
	data.alpha = m_ini->ReadInt(L"ControlWindow", L"alpha", 255);
	data.selectedItem = m_ini->ReadInt(L"Urls", L"selectedItem", 65536);
	data.urls = LoadUrls();

	return data;
}

void AppDataService::CreateDefaultConfig(const std::wstring& path)
{
	namespace fs = std::filesystem;

	if (fs::exists(path))
		return;

	HANDLE hFile = CreateFileW(
		path.c_str(),
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	// =========================
	// 默认数据（直接用 struct）
	// =========================
	MainWindowData mainData{};
	ControlWindowData controlData{};
	controlData.width = 900;
	controlData.height = 120;
	AppSettings settings{};
	// 1. 获取屏幕宽高
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int centerX = screenWidth / 2;
	int centerY = screenHeight / 2;

	// 2. 让 main 窗口的【几何中心】对齐屏幕中心
	// 公式：左上角坐标 = 屏幕中心 - 窗口宽高的一半
	mainData.x = centerX - (mainData.width / 2);
	mainData.y = centerY - (mainData.height / 2);

	// 3. 显示在 main 窗口的正上方
	// X 轴：与 main 窗口左对齐（如果你希望它们水平居中对齐，可以用：mainData.x + (mainData.width - controlData.width) / 2）
	controlData.x = mainData.x - 50;

	// Y 轴：main 窗口的顶边（mainData.y）减去 control 窗口自身的高度
	controlData.y = mainData.y - controlData.height - 100;
	// =========================
	// 写入
	// =========================
	SaveMainData(mainData);
	SaveControlData(controlData);
	SaveHotkeys(g_defaultHotkeys);
	SaveAppSettings(&settings);
}

bool AppDataService::ToggleHotkeysEnabled() noexcept
{
	m_hotkeysEnabled = !m_hotkeysEnabled;
	return m_hotkeysEnabled;
}

void AppDataService::SaveTheme(LocaleTheme theme)
{
	m_settings.theme = theme;
	int value = int(theme);
	SaveSettingItem(L"AppSettings", L"Theme", value);
}

bool AppDataService::ToggleSystemTray() noexcept
{
	m_settings.systemTrayExecute = !m_settings.systemTrayExecute;
	SaveSettingItem(L"AppSettings", L"SystemTrayExecute", m_settings.systemTrayExecute ? 1 : 0);
	return m_settings.systemTrayExecute;
}

bool AppDataService::ToggleRunAsAdmin() noexcept
{
	m_settings.adminRunning = !m_settings.adminRunning;
	SaveSettingItem(L"AppSettings", L"AdminRunning", m_settings.adminRunning ? 1 : 0);
	return m_settings.adminRunning;
}

bool AppDataService::ToggleAutoStart() noexcept
{
	m_settings.autoStart = !m_settings.autoStart;
	SaveSettingItem(L"AppSettings", L"AutoStart", m_settings.autoStart ? 1 : 0);
	return m_settings.autoStart;
}

bool AppDataService::ToggleIntelCpuUseEcore() noexcept
{
	m_settings.intelCpuUseECore = !m_settings.intelCpuUseECore;
	SaveSettingItem(L"AppSettings", L"IntelCpuUseECore", m_settings.intelCpuUseECore ? 1 : 0);
	return m_settings.intelCpuUseECore;
}

void AppDataService::SetMainWindowState(WindowState state) noexcept
{
	if (m_mainData.windowState == state)
		return;
	m_mainData.windowState = state;
	SaveSettingItem(L"MainWindow", L"windowState", (int)state);
}

void AppDataService::SaveLanguage(LocaleLanguage language)
{
	m_settings.language = language;
	SaveSettingItem(L"AppSettings", L"Language", (int)language);
}

void AppDataService::SaveInputMethod(::InputType type) noexcept
{
	m_settings.inputType = type;
	SaveSettingItem(L"AppSettings", L"InputType", (int)type);
	PipeService::Get().SendInputMethod(static_cast<uint32_t>(type));//通知服务端更改
}

void AppDataService::SaveHomePage(std::wstring_view url) noexcept
{
	m_settings.homePage = url;
	SaveSettingItem(L"AppSettings", L"HomePage", url);
}

void AppDataService::SaveKeyboardOff(bool value) noexcept
{
	m_settings.keyboardOff = value;
	PipeService::Get().SendFilterRuleSwitch(value);
	SaveSettingItem(L"AppSettings", L"KeyboardOff", value);
}

bool AppDataService::KeyboardOff(bool value) noexcept
{
	return m_settings.keyboardOff;
}

void AppDataService::SaveEnableWindowSnapping(bool value) noexcept
{
	m_settings.enableWindowSnapping = value;
	SaveSettingItem(L"AppSettings", L"EnableWindowSnapping", value);
}

