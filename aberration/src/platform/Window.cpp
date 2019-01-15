#include "Window.h"

#if defined(AB_PLATFORM_WINDOWS)
#include "windows/Win32Window.h"

namespace AB {
	static Win32::Window* WindowInfo = nullptr;

	AB_API void WindowCreate(const String& title, uint32 width, uint32 height) {
		if (WindowInfo != nullptr)
			AB_CORE_ERROR("Windows already created.");
		else {
			WindowInfo = Win32::WindowCreate(title, width, height);
			AB_CORE_ASSERT(WindowInfo, "Failed to create Win32 window.");
		}
	}

	AB_API void WindowPollEvents() {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::WindowPollEvents(WindowInfo);
	}

	AB_API bool WindowIsOpen() {
		if (!WindowInfo) {
			AB_CORE_WARN("Window is not created.");
			return false;
		}
		else {
			return Win32::WindowIsOpen(WindowInfo);
		}
	}

	AB_API void WindowGetSize(uint32& width, uint32& height) {
		if (!WindowInfo)
			AB_CORE_WARN("Window is not created.");
		else
			Win32::WindowGetSize(WindowInfo, width, height);
	}

	AB_API void WindowSetCloseCallback(const std::function<void()>& func) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::WindowSetCloseCallback(WindowInfo, func);
	}

	AB_API void WindowSetResizeCallback(const std::function<void(int, int)>& func) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::WindowSetResizeCallback(WindowInfo, func);
	}

	AB_API void WindowDestroy() {
		if (!WindowInfo)
			AB_CORE_WARN("Window is not created.");
		else {
			Win32::WindowDestroyWindow(WindowInfo);
			WindowInfo = nullptr;
		}
	}

	AB_API bool InputGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button) {
		if (!WindowInfo) {
			AB_CORE_WARN("Window is not created.");
			return false;
		}
		else {
			return Win32::InputGamepadButtonPressed(WindowInfo, gamepadNumber, button);
		}
	}

	AB_API bool InputGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button) {
		if (!WindowInfo) {
			AB_CORE_WARN("Window is not created.");
			return false;
		}
		else {
			return Win32::InputGamepadButtonReleased(WindowInfo, gamepadNumber, button);
		}
	}

	AB_API bool InputGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button) {
		if (!WindowInfo) {
			AB_CORE_WARN("Window is not created.");
			return false;
		}
		else {
			return Win32::InputGamepadButtonHeld(WindowInfo, gamepadNumber, button);
		}
	}

	AB_API void InputSetGamepadButtonCallback(const std::function<void(uint8, GamepadButton, bool, bool)>& func) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::InputSetGamepadButtonCallback(WindowInfo, func);
	}

	AB_API void InputGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::InputGetGamepadStickPosition(WindowInfo, gamepadNumber, leftX, leftY, rightX, rightY);
	}

	AB_API void InputGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::InputGetGamepadStickPosition(WindowInfo, gamepadNumber, lt, rt);
	}

	AB_API void InputSetGamepadStickCallback(const std::function<void(uint8, int16, int16, int16, int16)>& func) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::InputSetGamepadStickCallback(WindowInfo, func);
	}

	AB_API void InputSetGamepadTriggerCallback(const std::function<void(uint8, byte, byte)>& func) {
		if (!WindowInfo)
			AB_CORE_ERROR("Window is not created.");
		else
			Win32::InputSetGamepadTriggerCallback(WindowInfo, func);
	}

}
#endif
