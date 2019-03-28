#pragma once
#include "AB.h"
#include "Input.h"

namespace AB {
	typedef void(PlatformCloseCallback)(void);
	typedef void(PlatformResizeCallback)(uint32 width, uint32 height);
	typedef void(PlatformGamepadButtonCallback)(uint8 gpNumber, GamepadButton btn, bool32 currState, bool32 prevState);
	typedef void(PlatformGamepadStickCallback)(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs);
	typedef void(PlatformGamepadTriggerCallback)(uint8 gpNumber, byte lt, byte rt);

	void WindowCreate(const char* title, uint32 width, uint32 height);
	void WindowDestroy();
	void WindowClose();
	bool WindowIsOpen();
	void WindowPollEvents();
	void WindowSwapBuffers();
	void WindowEnableVSync(bool32 enable);

	void WindowGetSize(uint32* width, uint32* height);
	void WindowSetMousePosition(uint32 x, uint32 y);
	bool32 WindowActive();
	void WindowShowCursor(bool32 show);

	void WindowSetCloseCallback(PlatformCloseCallback* func);
	void WindowSetResizeCallback(PlatformResizeCallback* func);

	bool WindowMouseInClientArea();

	// Currently not working on Linux
	bool WindowGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
	bool WindowGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
	bool WindowGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
	void WindowGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
	void WindowGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
	void WindowSetGamepadButtonCallback(PlatformGamepadButtonCallback* func);
	void WindowSetGamepadStickCallback(PlatformGamepadStickCallback* func);
	void WindowSetGamepadTriggerCallback(PlatformGamepadTriggerCallback* func);
};

