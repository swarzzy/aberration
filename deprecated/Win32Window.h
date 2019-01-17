//#pragma once
//#include "src/ABHeader.h"
//#include "../Gamepad.h"
//#include "src/utils/Log.h"
//
//#include <Windows.h>
//#include <Xinput.h>
//
//// TODO: Gamepad deadzones
//
//namespace AB::Win32 {
//
//
//	struct GamepadAnalogCtrl {
//		int16 leftStickX;
//		int16 leftStickY;
//		int16 rightStickX;
//		int16 rightStickY;
//		byte leftTrigger;
//		byte rightTrigger;
//		uint16 leftStickDeadZone;
//		uint16 rightStickDeadZone;
//		byte triggerDeadZone;
//	};
//
//	struct Win32Key {
//		bool currentState;
//		bool prevState;
//		uint32 repeatCount;
//	};
//
//	struct Window {
//		HWND windowHandle;
//		HDC windowDC;
//		uint32 width;
//		uint32 height;
//		// TODO: This string calling delete with nullptr for some reason
//		String title;
//		bool running;
//		std::function<void()> closeCallback;
//		// TODO: Use sized types
//		std::function<void(int, int)> resizeCallback;
//		std::function<void(uint8, GamepadButton, bool, bool)> gamepadButtonCallback;
//		// TODO: Size const
//		bool gamepadsCurrentState[GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT];
//		bool gamepadsPrevState[GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT];
//		GamepadAnalogCtrl gamepadsAnalogControls[XUSER_MAX_COUNT];
//		std::function<void(uint8, int16, int16, int16, int16)> gamepadStickCallback;
//		std::function<void(uint8, byte, byte)> gamepadTriggerCallback;
//		// MOUSE
//		int32 mousePositionX;
//		int32 mousePositionY;
//		std::function<void(MouseButton, bool)> mouseButtonCallback;
//		std::function<void(uint32, uint32)> mouseMoveCallback;
//		bool mouseButtonsCurrentState[MOUSE_BUTTONS_COUNT];
//		bool mouseInClientArea;
//		bool mouseCaptured;
//		TRACKMOUSEEVENT mouseWin32TrackEvent;
//		// KEYBOARD
//		Win32Key keys[KEYBOARD_KEYS_COUNT]; // 1-current state, 2-prev state, 3+ - repeat count
//		std::function<void(KeyboardKey, bool, bool, uint32)> keyCallback;
//	};
//
//	// TODO: Do we need AB_API for that
//	// TODO: Does passing Window pointer around makes sense?
//	AB_API Window* WindowCreate(const String& title, uint32 width, uint32 height);
//	AB_API void WindowPollEvents(Window* window);
//	AB_API bool WindowIsOpen(const Window* window);
//	AB_API void WindowGetSize(const Window* window, uint32& width, uint32& height);
//	AB_API void WindowSetCloseCallback(Window* window, const std::function<void()>& func);
//	AB_API void WindowSetResizeCallback(Window* window, const std::function<void(int, int)>& func);
//	AB_API void WindowDestroyWindow(Window* window); // Sets window to nullptr
//	//AB_API Window* WindowGetWindow()
//	LRESULT CALLBACK WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
//
//	AB_API void LoadXInput();
//	AB_API void GamepadUpdate(Window* window);
//	AB_API bool InputGamepadButtonPressed(const Window* window, uint8 gamepadNumber, GamepadButton button);
//	AB_API bool InputGamepadButtonReleased(const Window* window, uint8 gamepadNumber, GamepadButton button);
//	AB_API bool InputGamepadButtonHeld(const Window* window, uint8 gamepadNumber, GamepadButton button);
//	AB_API void InputSetGamepadButtonCallback(Window* window, const std::function<void(uint8, GamepadButton, bool, bool)>& func);
//	AB_API void InputGetGamepadStickPosition(Window* window, uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
//	AB_API void InputGetGamepadStickPosition(Window* window, uint8 gamepadNumber, byte& lt, byte& rt);
//	AB_API void InputSetGamepadStickCallback(Window* window, const std::function<void(uint8, int16, int16, int16, int16)>& func);
//	AB_API void InputSetGamepadTriggerCallback(Window* window, const std::function<void(uint8, byte, byte)>& func);
//
//	AB_API void InputGetMousePosition(Window* window, int32& xPos, int32& yPos);
//	AB_API bool InputMouseButtonPressed(Window* window, MouseButton button);
//	AB_API bool InputMouseInClientArea(Window* window);
//	AB_API void InputMouseCapture(Window* window, bool capture);
//	AB_API void InputSetMouseButtonCallback(Window* window, const std::function<void(MouseButton, bool)>& func);
//	AB_API void InputSetMouseMoveCallback(Window* window, const std::function<void(uint32, uint32)>& func);
//
//	AB_API bool InputKeyPressed(Window* window, KeyboardKey key);
//	AB_API void InputKeySetCallback(Window* window, const std::function<void(KeyboardKey, bool, bool, uint32)>& func);
//
//}
