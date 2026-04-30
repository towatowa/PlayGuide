#pragma once
// RawInputManager.h
#pragma once
#include <functional>
#include <atomic>
#include "KeyEvent.h"
#include <Windows.h>
#include "InputEvent.h"

struct RawInputConfig {
	USHORT usagePage = 0x01;
	USHORT usage = 0x06;   // keyboard

	bool inputSink = true;   // RIDEV_INPUTSINK
	bool noLegacy = false;  // RIDEV_NOLEGACY
	bool captureAll = false;  // RIDEV_CAPTUREMOUSE 等

	HWND targetWindow = nullptr;
};

class RawInputManager final : public InputEvent {
public:
	RawInputManager();
	~RawInputManager();

	bool Start() override;
	void Stop() override;
	void SetConfig(const RawInputConfig& cfg) {
		m_config = cfg;
	}
	void AttachThreadInput(HWND hWnd) override;

private:
	void MessageThread();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	RAWINPUTDEVICE BuildRID() const;
	void HandleRawInput(HRAWINPUT hRaw);
private:
	std::atomic_bool m_running{ false };
	std::thread m_thread;
	DWORD m_threadId{};
	HWND m_hwnd{ nullptr };
	RawInputConfig m_config;
	static RawInputManager* s_instance;
};
