#pragma once
#include <cstdint>
#include <Windows.h>

enum class InputSource {
	KeyboardHook,
	RawInput
};
enum class OptType
{
	KeyDown = 0,
	KeyUp,
	KeyTap,
	Delay
};
class KeyEvent
{
public:
	USHORT vk;
	DWORD scanCode{};
	OptType action{};
	DWORD time{};
	HWND sourceWindow{};

	// 输入来源
	virtual InputSource Source() const = 0;

	// 语义字段
	bool injected{};
	bool repeat{};
	bool isModifier{};

	// 位置 / 扩展
	bool extended{};
};

class KeyboardHookEvent : public KeyEvent {
public:
	InputSource Source() const override {
		return InputSource::KeyboardHook;
	}
};

class RawInputEvent : public KeyEvent {
public:
	HANDLE device{};
	uint16_t rawFlags{};

	InputSource Source() const override {
		return InputSource::RawInput;
	}
};
