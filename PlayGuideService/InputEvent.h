#pragma once
#include <functional>
#include <thread>
#include <Windows.h>
#include "KeyEvent.h"
#include "Appdata.h"

class InputEvent {
public:
	static const uint8_t VK_COUNT = 256;
	using Callback = std::function<void(SimpleEvent& ev)>;

	virtual ~InputEvent() = default;

	virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual void SetCallback(Callback cb) {
		m_callback = std::move(cb);
	}
	virtual void AttachThreadInput(HWND hWnd) = 0;
protected:
	Callback m_callback;
};
