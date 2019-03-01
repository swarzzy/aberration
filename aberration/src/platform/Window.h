#pragma once
#include "ABHeader.h"
#include "Input.h"

namespace AB {
	typedef void(PlatformCloseCallback)(void);
	typedef void(PlatformResizeCallback)(uint32 width, uint32 height);
	typedef void(PlatformFocusCallback)(bool32 focus, void* userData);
	typedef void(PlatformKeyCallback)(KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount);
	typedef void(PlatformMouseButtonCallback)(MouseButton btn, bool32 state);
	typedef void(PlatformMouseMoveCallback)(uint32 xPos, uint32 yPos, void* userData);
	typedef void(PlatformGamepadButtonCallback)(uint8 gpNumber, GamepadButton btn, bool32 currState, bool32 prevState);
	typedef void(PlatformGamepadStickCallback)(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs);
	typedef void(PlatformGamepadTriggerCallback)(uint8 gpNumber, byte lt, byte rt);

	struct PlatformKeyInfo {
		byte currentState;
		byte prevState;
	};

	struct PlatformMouseButtonInfo {
		byte currentState;
		byte prevState;
	};

	class AB_API Window final {
		AB_DISALLOW_COPY_AND_MOVE(Window)
	private:
		inline static struct AB_API WindowProperties* s_WindowProperties = nullptr;

		Window() = delete;
		~Window();

	public:
		static void Create(const char* title, uint32 width, uint32 height);
		static void Destroy();
		static void Close();
		static bool IsOpen();
		static void PollEvents();
		static void SwapBuffers();
		static void EnableVSync(bool32 enable);

		static PlatformKeyInfo GetKeyState(KeyboardKey key);
		static PlatformMouseButtonInfo GetMouseButtonState(MouseButton button);
		static void SetMouseMoveCallback(PlatformMouseMoveCallback* func, void* userData);
		static void GetSize(uint32* width, uint32* height);
		static void SetMousePosition(uint32 x, uint32 y);
		static void SetFocusCallback(void* userData, PlatformFocusCallback* func);
		static bool32 WindowActive();
		static void ShowCursor(bool32 show);

		static void SetCloseCallback(PlatformCloseCallback* func);
		static void SetResizeCallback(PlatformResizeCallback* func);

		//static bool32 GetKeyboardState(byte* buffer, uint32 bufferSize);

		static bool KeyPressed(KeyboardKey key);
		static void SetKeyCallback(PlatformKeyCallback* func);

		
		static void GetMousePosition(uint32* xPos, uint32* yPos);
		static bool MouseButtonPressed(MouseButton button);
		static bool MouseInClientArea();
		static void SetMouseButtonCallback(PlatformMouseButtonCallback* func);
		


		// Currently not working on Linux
		static bool GamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
		static bool GamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
		static bool GamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
		static void GetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
		static void GetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
		static void SetGamepadButtonCallback(PlatformGamepadButtonCallback* func);
		static void SetGamepadStickCallback(PlatformGamepadStickCallback* func);
		static void SetGamepadTriggerCallback(PlatformGamepadTriggerCallback* func);

	private:
		static void PlatformCreate();
	};
}
