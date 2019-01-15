#pragma once
#include "src/ABHeader.h"
#include "Gamepad.h"

namespace AB {

	AB_API void WindowCreate(const String& title, uint32 width, uint32 height);
	AB_API void WindowPollEvents();
	AB_API bool WindowIsOpen();
	AB_API void WindowGetSize(uint32& width, uint32& height);
	AB_API void WindowSetCloseCallback(const std::function<void()>& func);
	AB_API void WindowSetResizeCallback(const std::function<void(int, int)>& func);
	AB_API void WindowDestroy();

	AB_API bool InputGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
	AB_API bool InputGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
	AB_API bool InputGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
	AB_API void InputSetGamepadButtonCallback(const std::function<void(uint8, GamepadButton, bool, bool)>& func); // gamepad no, button, current, prev
	AB_API void InputGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
	AB_API void InputGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
	AB_API void InputSetGamepadStickCallback(const std::function<void(uint8, int16, int16, int16, int16)>& func); // gamepad lsX lsY rsX rsY
	AB_API void InputSetGamepadTriggerCallback(const std::function<void(uint8, byte, byte)>& func); // gamepad lt rt
}
