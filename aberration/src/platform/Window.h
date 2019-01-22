#pragma once
#include "src/ABHeader.h"
#include "Input.h"

namespace AB {
	class AB_API Window final {
		AB_DISALLOW_COPY_AND_MOVE(Window)
	private:
		inline static struct AB_API WindowProperties* s_WindowProperties = nullptr;

		Window() = delete;
		~Window();

	public:
		static void Create(const String& title, uint32 width, uint32 height);
		static void Destroy();
		static bool IsOpen();
		static void PollEvents();

		static void GetSize(uint32& width, uint32& height);
		static void SetCloseCallback(const std::function<void()>& func);
		static void SetResizeCallback(const std::function<void(uint32 width, uint32 height)>& func);

		static bool KeyPressed(KeyboardKey key);
		static void SetKeyCallback(const std::function<void(KeyboardKey key, bool currState, bool prevState, uint32 repeatCount)>& func);

		static void GetMousePosition(uint32& xPos, uint32& yPos);
		static bool MouseButtonPressed(MouseButton button);
		static bool MouseInClientArea();
		static void SetMouseButtonCallback(const std::function<void(MouseButton btn, bool state)>& func);
		static void SetMouseMoveCallback(const std::function<void(uint32 xPos, uint32 yPos)>& func);


		// Currently not working on Linux
		static bool GamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
		static bool GamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
		static bool GamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
		static void GetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
		static void GetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
		static void SetGamepadButtonCallback(const std::function<void(uint8 gpNumber, GamepadButton btn, bool currState, bool prevState)>& func);
		static void SetGamepadStickCallback(const std::function<void(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs)>& func);
		static void SetGamepadTriggerCallback(const std::function<void(uint8 gpNumber, byte lt, byte rt)>& func);

	private:
		static void PlatformCreate();
	};
}