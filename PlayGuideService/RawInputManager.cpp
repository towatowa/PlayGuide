#include <vector>
#include "RawInputManager.h"

RawInputManager* RawInputManager::s_instance = nullptr;
RawInputManager::RawInputManager()
{
}

RawInputManager::~RawInputManager()
{
	Stop();
}

bool RawInputManager::Start()
{
	if (m_running.load())
		return true;
	
	if (s_instance)
		return false;

	s_instance = this;
	m_running.store(true);

	m_thread = std::thread(&RawInputManager::MessageThread, this);

	// 等待窗口创建完成
	while (!m_hwnd)
		Sleep(1);
	auto rid = BuildRID();
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
	return true;
}

void RawInputManager::Stop()
{
	if (!m_running.exchange(false))
		return;

	if (m_threadId)
		PostThreadMessage(m_threadId, WM_QUIT, 0, 0);

	if (m_thread.joinable())
		m_thread.join();

	m_hwnd = nullptr;
	s_instance = nullptr;
}

void RawInputManager::AttachThreadInput(HWND hWnd)
{
	if (m_threadId == 0)
		return;
	DWORD targetThreadId = GetWindowThreadProcessId(hWnd, nullptr);
	if (targetThreadId == 0)
		return;
	if (!::AttachThreadInput(m_threadId, targetThreadId, TRUE))
	{
		OutputDebugStringW(L"RawInputManager::AttachThreadInput failed\n");
		return;
	}
	return;
}

void RawInputManager::MessageThread()
{
	m_threadId = GetCurrentThreadId();

	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpszClassName = L"RawInputHiddenWindow";

	RegisterClassExW(&wc);

	m_hwnd = CreateWindowExW(
		0,
		wc.lpszClassName,
		L"",
		0,
		0, 0, 0, 0,
		HWND_MESSAGE,
		nullptr,
		wc.hInstance,
		nullptr
	);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(m_hwnd);
}

void RawInputManager::HandleRawInput(HRAWINPUT hRaw)
{
	if (!m_running.load())
		return;

	UINT size = 0;
	GetRawInputData(hRaw, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

	std::vector<BYTE> buffer(size);
	if (GetRawInputData(hRaw, RID_INPUT, buffer.data(), &size,
		sizeof(RAWINPUTHEADER)) != size)
		return;

	auto* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
	if (raw->header.dwType != RIM_TYPEKEYBOARD)
		return;

	const auto& kb = raw->data.keyboard;

	SimpleEvent ev{};
	ev.vk = kb.VKey;
	ev.hwnd= GetForegroundWindow();
	ev.hwnd = GetAncestor(ev.hwnd, GA_ROOT); // 获取顶层窗口
	if (m_callback)
		m_callback(ev);
}

LRESULT CALLBACK RawInputManager::WndProc(
	HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_INPUT && s_instance)
	{
		s_instance->HandleRawInput(reinterpret_cast<HRAWINPUT>(lp));
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

RAWINPUTDEVICE RawInputManager::BuildRID() const
{
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = m_config.usagePage;
	rid.usUsage = m_config.usage;
	rid.hwndTarget = m_hwnd;

	if (m_config.inputSink)
		rid.dwFlags |= RIDEV_INPUTSINK;
	if (m_config.noLegacy)
		rid.dwFlags |= RIDEV_NOLEGACY;
	if (m_config.captureAll)
		rid.dwFlags |= RIDEV_CAPTUREMOUSE;

	return rid;
}