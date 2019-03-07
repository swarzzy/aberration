#pragma once
#include "AB.h"
#include "Input.h"

namespace AB {
	typedef void(PlatformCloseCallback)(void);
	typedef void(PlatformResizeCallback)(uint32 width, uint32 height);
	//typedef void(PlatformFocusCallback)(bool32 focus, void* userData);
	//typedef void(PlatformKeyCallback)(KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount);
	//typedef void(PlatformMouseButtonCallback)(MouseButton btn, bool32 state);
	//typedef void(PlatformMouseMoveCallback)(uint32 xPos, uint32 yPos, void* userData);
	typedef void(PlatformGamepadButtonCallback)(uint8 gpNumber, GamepadButton btn, bool32 currState, bool32 prevState);
	typedef void(PlatformGamepadStickCallback)(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs);
	typedef void(PlatformGamepadTriggerCallback)(uint8 gpNumber, byte lt, byte rt);

	//struct PlatformKeyInfo {
	//	byte currentState;
	//	byte prevState;
	//};

	namespace Window{
		void Create(const char* title, uint32 width, uint32 height);
		void Destroy();
		void Close();
		bool IsOpen();
		void PollEvents();
		void SwapBuffers();
		void EnableVSync(bool32 enable);

		//PlatformKeyInfo GetKeyState(KeyboardKey key);
		//PlatformMouseButtonInfo GetMouseButtonState(MouseButton button);
		void GetSize(uint32* width, uint32* height);
		void SetMousePosition(uint32 x, uint32 y);
		//void SetFocusCallback(void* userData, PlatformFocusCallback* func);
		bool32 WindowActive();
		void ShowCursor(bool32 show);

		void SetCloseCallback(PlatformCloseCallback* func);
		void SetResizeCallback(PlatformResizeCallback* func);

		//static bool32 GetKeyboardState(byte* buffer, uint32 bufferSize);

		//bool KeyPressed(KeyboardKey key);
		//void SetKeyCallback(PlatformKeyCallback* func);

		
		//void GetMousePosition(uint32* xPos, uint32* yPos);
		//bool MouseButtonPressed(MouseButton button);
		bool MouseInClientArea();
		//void SetMouseButtonCallback(PlatformMouseButtonCallback* func);
		


		// Currently not working on Linux
		bool GamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
		bool GamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
		bool GamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
		void GetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
		void GetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
		void SetGamepadButtonCallback(PlatformGamepadButtonCallback* func);
		void SetGamepadStickCallback(PlatformGamepadStickCallback* func);
		void SetGamepadTriggerCallback(PlatformGamepadTriggerCallback* func);
	};
}
