#include "KeyboardHook.h"
#include "Appdata.h"
#include <future>
#include <thread>
#include "KeyStateTracker.h"

KeyboardHook* KeyboardHook::s_instance = nullptr;
KeyboardHook::~KeyboardHook()
{
	Stop();
}

void KeyboardHook::HookThread()
{
	m_threadId = GetCurrentThreadId();

	m_hook = SetWindowsHookExW(
		WH_KEYBOARD_LL,
		HookProc,
		GetModuleHandleW(nullptr),
		0
	);

	if (!m_hook)
		return;

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(m_hook);
	m_hook = nullptr;
}

void KeyboardHook::AttachThreadInput(HWND hWnd)
{
	if (m_threadId == 0)
		return;
	DWORD targetThreadId = GetWindowThreadProcessId(hWnd, nullptr);
	if (targetThreadId == 0)
		return;
	if (!::AttachThreadInput(m_threadId, targetThreadId, TRUE))
	{
		OutputDebugStringW(L"KeyboardHook::AttachThreadInput failed\n");
		return;
	}
	return;
}

bool KeyboardHook::Start()
{
	if (m_hook)
		return true;

	if (s_instance)
		return false;

	s_instance = this;

	m_thread = std::thread(&KeyboardHook::HookThread, this);

	// 等待 hook 安装完成（简单做法）
	while (!m_hook)
		Sleep(1);

	return m_hook != nullptr;
}

void KeyboardHook::Stop()
{
	if (!m_hook)
		return;

	PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
	if (m_thread.joinable())
		m_thread.join();
	m_hook = nullptr;
	if (s_instance == this)
		s_instance = nullptr;
}

LRESULT CALLBACK KeyboardHook::HookProc(int code, WPARAM wp, LPARAM lp)
{
	
	if (code != HC_ACTION || !s_instance)
		return CallNextHookEx(nullptr, code, wp, lp);

	auto* info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lp);

	SimpleEvent ev;
	ev.vk = static_cast<USHORT>(info->vkCode);
	ev.action = (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN) ? KeyAction::KeyDown : KeyAction::KeyUp;
	//ev.hwnd = GetForegroundWindow();
	//ev.hwnd = GetAncestor(ev.hwnd, GA_ROOT); // 获取顶层窗口
    
	// ⚠ Hook 回调里只做最轻逻辑
	if (s_instance->m_callback) {
           s_instance->m_callback(ev);
	}
	return CallNextHookEx(nullptr, code, wp, lp);
}