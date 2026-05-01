#include "AppDataService.h"

void AppDataService::Initialize(const std::wstring& path)
{
	m_ini = std::make_unique<IniHelper>(path);

	CreateDefaultConfig(path);

	// 预加载
	m_mainData = LoadMainData();
	m_controlData = LoadControlData();
	m_hotkeyMap = LoadHotkeys();

	m_hotkey =
	{
		{ m_hotkeyMap[L"Opacity_Add"],         WM_OPACITY_ADD},   // OPACITY_ADD
		{ m_hotkeyMap[L"Opacity_Dec"],         WM_OPACITY_DEC },   // OPACITY_DEC
		{ m_hotkeyMap[L"Play"],                WM_PLAY, },       // Play
		{ m_hotkeyMap[L"Seek_Add"],            WM_SEEK_ADD },   // SeekAdd
        { m_hotkeyMap[L"Seek_Dec"],            WM_SEEK_DEC},   // SeekDec
		{ m_hotkeyMap[L"Show_Hide_Window"],    WM_SHOW_HIDE_WINDOW }    // Show or hide window
	};
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

	m_ini->WriteString(L"MainWindow", L"maximized",
		std::to_wstring(data.maximized));
	m_ini->WriteString(L"Web", L"url", data.url);
	//SaveUrls(data.urls);
}

void AppDataService::SaveHotkeys(const HotKeyMap& hotkeys)
{
	for (auto& [key, value] : hotkeys)
	{
		m_ini->WriteString(
			L"Hotkey",
			key,
			std::to_wstring(value)
		);
	}
}

void AppDataService::SaveUrls(const std::vector<std::wstring>& data)
{
	m_ini->WriteInt(L"Urls", L"urlCount", data.size());
	std::wstring key;
	for (int i = 0; i < data.size(); ++i)
	{
		std::wstring key = L"url" + std::to_wstring(i);
		m_ini->WriteString(L"Urls", key, data[i]);
	}
}

HotKeyMap AppDataService::LoadHotkeys() const
{
	HotKeyMap map;

	std::wstring keys[] = {
		L"Opacity_Add",
		L"Opacity_Dec",
		L"Play",
		L"Seek_Add",
		L"Seek_Dec",
		L"Show_Hide_Window"
	};

	for (auto& k : keys)
	{
		UINT val = m_ini->ReadInt(L"Hotkey", k, 0);

		map[k] = val;
	}

	return map;
}

std::vector<std::wstring> AppDataService::LoadUrls() const
{
	// vector webUrls
	int count = m_ini->ReadInt(L"Urls", L"urlCount", 1);
	std::vector<std::wstring>urls;

	for (int i = 0; i < count; i++)
	{
		std::wstring key = L"url" + std::to_wstring(i);

		auto url = m_ini->ReadString(L"Urls", key, L"");

		if (url[0])
			urls.emplace_back(url);
	}

	return urls;
}

MainWindowData AppDataService::LoadMainData() const
{
	MainWindowData data;

	data.x = m_ini->ReadInt(L"MainWindow", L"x", 100);
	data.y = m_ini->ReadInt(L"MainWindow", L"y", 100);
	data.width = m_ini->ReadInt(L"MainWindow", L"width", 1280);
	data.height = m_ini->ReadInt(L"MainWindow", L"height", 720);
	data.alpha = m_ini->ReadInt(L"MainWindow", L"alpha", 255);
	data.maximized = m_ini->ReadInt(L"MainWindow", L"maximized", 0);

	data.url = m_ini->ReadString(L"Web", L"url", g_defaultWebUrl);

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
}

ControlWindowData AppDataService::LoadControlData() const
{
	ControlWindowData data;

	data.x = m_ini->ReadInt(L"ControlWindow", L"x", 100);
	data.y = m_ini->ReadInt(L"ControlWindow", L"y", 100);
	data.width = m_ini->ReadInt(L"ControlWindow", L"width", 1280);
	data.height = m_ini->ReadInt(L"ControlWindow", L"height", 720);
	data.alpha = m_ini->ReadInt(L"ControlWindow", L"alpha", 255);

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
	mainData.url = g_defaultWebUrl;
	ControlWindowData controlData{};
	controlData.width = 854;
	controlData.height = 120;
	// =========================
	// 写入
	// =========================
	SaveMainData(mainData);
	SaveControlData(controlData);
	SaveHotkeys(g_defaultHotkeys);
}