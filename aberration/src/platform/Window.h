#pragma once
#include "src/ABHeader.h"
#include "Input.h"

namespace AB {
	typedef void(CloseCallback)(void);
	typedef void(ResizeCallback)(uint32 width, uint32 height);
	typedef void(KeyCallback)(KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount);
	typedef void(MouseButtonCallback)(MouseButton btn, bool32 state);
	typedef void(MouseMoveCallback)(uint32 xPos, uint32 yPos);
	typedef void(GamepadButtonCallback)(uint8 gpNumber, GamepadButton btn, bool32 currState, bool32 prevState);
	typedef void(GamepadStickCallback)(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs);
	typedef void(GamepadTriggerCallback)(uint8 gpNumber, byte lt, byte rt);

	class AB_API Window final {
		AB_DISALLOW_COPY_AND_MOVE(Window)
	private:
		inline static struct AB_API WindowProperties* s_WindowProperties = nullptr;

		Window() = delete;
		~Window();

	public:
		static void Create(const String& title, uint32 width, uint32 height);
		static void Destroy();
		static void Close();
		static bool IsOpen();
		static void PollEvents();
		static void SwapBuffers();
		static void EnableVSync(bool32 enable);

		static void GetSize(uint32& width, uint32& height);
		static void SetCloseCallback(CloseCallback* func);
		static void SetResizeCallback(ResizeCallback* func);

		static bool KeyPressed(KeyboardKey key);
		static void SetKeyCallback(KeyCallback* func);

		static void GetMousePosition(uint32& xPos, uint32& yPos);
		static bool MouseButtonPressed(MouseButton button);
		static bool MouseInClientArea();
		static void SetMouseButtonCallback(MouseButtonCallback* func);
		static void SetMouseMoveCallback(MouseMoveCallback* func);


		// Currently not working on Linux
		static bool GamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
		static bool GamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
		static bool GamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
		static void GetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
		static void GetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
		static void SetGamepadButtonCallback(GamepadButtonCallback* func);
		static void SetGamepadStickCallback(GamepadStickCallback* func);
		static void SetGamepadTriggerCallback(GamepadTriggerCallback* func);

	private:
		static void PlatformCreate();
	};
}