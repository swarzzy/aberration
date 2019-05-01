#pragma once
#include "AB.h"
#include "Input.h"
#include "Shared.h"

namespace AB {

	struct InputMgr;
	struct WindowProperties;
	struct MemoryArena;

	
	
	typedef void(PlatformCloseCallback)(void);
	typedef void(PlatformResizeCallback)(uint32 width, uint32 height);
	typedef void(PlatformGamepadButtonCallback)(uint8 gpNumber, GamepadButton btn, bool32 currState, bool32 prevState);
	typedef void(PlatformGamepadStickCallback)(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs);
	typedef void(PlatformGamepadTriggerCallback)(uint8 gpNumber, byte lt, byte rt);

	WindowProperties* WindowAllocateAndInit(MemoryArena* arena,
											const char* title,
											uint32 width,
											uint32 height);
	void WindowDestroy(WindowProperties* window);
	void WindowClose(WindowProperties* window);
	bool WindowIsOpen(WindowProperties* window);
	void WindowPollEvents(WindowProperties* window);
	void WindowSwapBuffers(WindowProperties* window);
	void WindowEnableVSync(WindowProperties* window, bool32 enable);
	void WindowRegisterInputManager(WindowProperties* window,
									void* inputManager,
									PlatformInputCallbacks* callbacks);
	void WindowGetSize(WindowProperties* window, uint32* width, uint32* height);
	void WindowSetMousePosition(WindowProperties* window, uint32 x, uint32 y);
	bool32 WindowActive(WindowProperties* window);
	void WindowShowCursor(WindowProperties* window, bool32 show);

	void WindowSetCloseCallback(WindowProperties* window, PlatformCloseCallback* func);
	void WindowSetResizeCallback(WindowProperties* window, PlatformResizeCallback* func);

	#if 0
	//	bool WindowMouseInClientArea();

	// Currently not working on Linux
	bool WindowGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button);
	bool WindowGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button);
	bool WindowGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button);
	void WindowGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY);
	void WindowGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt);
	void WindowSetGamepadButtonCallback(PlatformGamepadButtonCallback* func);
	void WindowSetGamepadStickCallback(PlatformGamepadStickCallback* func);
	void WindowSetGamepadTriggerCallback(PlatformGamepadTriggerCallback* func);
	#endif
};

