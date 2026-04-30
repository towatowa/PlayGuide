#pragma once
#include <bitset>
#include <Windows.h>
#include <functional>
#include <set>
#include <thread>

#include "KeyEvent.h"
#include "InputEvent.h"

class KeyboardHook final : public InputEvent
{
public:
	~KeyboardHook();
	bool Start() override;
	void Stop() override;
	void HookThread();
	void AttachThreadInput(HWND hWnd) override;
private:
	static LRESULT CALLBACK HookProc(int code, WPARAM wp, LPARAM lp);
private:
	std::thread m_thread;
	DWORD m_threadId{ 0 };
	HHOOK m_hook{ nullptr };
	static KeyboardHook* s_instance;
};
