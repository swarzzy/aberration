#pragma once
#include "src/ABHeader.h"
#include "../Gamepad.h"

#include <Windows.h>
#include <Xinput.h>

// TODO: Gamepad deadzones

namespace AB::Win32 {

	static const char* AB_XINPUT_DLL = "xinput1_3.dll";
	static const char* WINDOW_CLASS_NAME = "Aberration Engine Win32";

	typedef DWORD WINAPI _Win32XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
	typedef DWORD WINAPI _Win32XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

	// Dummy functions for use if XInput loading failed. Is will work as is no device connected
	inline static DWORD WINAPI _Win32XInputGetStateDummy(DWORD dwUserIndex, XINPUT_STATE* pState) {
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	inline static DWORD WINAPI _Win32XInputSetStateDummy(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	static _Win32XInputGetState* Win32XInputGetState = _Win32XInputGetStateDummy;
	static _Win32XInputSetState* Win32XInputSetState = _Win32XInputSetStateDummy;

	struct GamepadAnalogCtrl {
		int16 leftStickX;
		int16 leftStickY;
		int16 rightStickX;
		int16 rightStickY;
		byte leftTrigger;
		byte rightTrigger;
		uint16 leftStickDeadZone;
		uint16 rightStickDeadZone;
		byte triggerDeadZone;
	};

	struct Window {
		HWND windowHandle;
		uint32 width;
		uint32 height;
		// TODO: This string calling delete with nullptr for some reason
		String title;
		bool running;
		std::function<void()> closeCallback;
		// TODO: Use sized types
		std::function<void(int, int)> resizeCallback;
		std::function<void(uint8, GamepadButton, bool, bool)> gamepadButtonCallback;
		// TODO: Size const
		bool gamepadsCurrentState[GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT];
		bool gamepadsPrevState[GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT];
		GamepadAnalogCtrl gamepadsAnalogControls[XUSER_MAX_COUNT];
		std::function<void(uint8, int16, int16, int16, int16)> gamepadStickCallback;
		std::function<void(uint8, byte, byte)> gamepadTriggerCallback;
	};

	// TODO: Do we need AB_API for that
	// TODO: Does passing Window pointer around makes sense?
	AB_API Window* WindowCreate(const String& title, uint32 width, uint32 height);
	AB_API void WindowPollEvents(Window* window);
	AB_API bool WindowIsOpen(const Window* window);
	AB_API void WindowGetSize(const Window* window, uint32& width, uint32& height);
	AB_API void WindowSetCloseCallback(Window* window, const std::function<void()>& func);
	AB_API void WindowSetResizeCallback(Window* window, const std::function<void(int, int)>& func);
	AB_API void WindowDestroyWindow(Window* window); // Sets window to nullptr
	//AB_API Window* WindowGetWindow()
	LRESULT CALLBACK WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

	AB_API void LoadXInput();
	AB_API void GamepadUpdate(Window* window);
	AB_API bool InputGamepadButtonPressed(const Window* window, uint8 gamepadNumber, GamepadButton button);
	AB_API bool InputGamepadButtonReleased(const Window* window, uint8 gamepadNumber, GamepadButton button);
	AB_API bool InputGamepadButtonHeld(const Window* window, uint8 gamepadNumber, GamepadButton button);
	AB_API void InputSetGamepadButtonCallback(Window* window, const std::function<void(uint8, GamepadButton, bool, bool)>& func);
	AB_API void InputGetGamepadStickPosition(Window* window, uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
	AB_API void InputGetGamepadStickPosition(Window* window, uint8 gamepadNumber, byte& lt, byte& rt);
	AB_API void InputSetGamepadStickCallback(Window* window, const std::function<void(uint8, int16, int16, int16, int16)>& func);
	AB_API void InputSetGamepadTriggerCallback(Window* window, const std::function<void(uint8, byte, byte)>& func);
}
