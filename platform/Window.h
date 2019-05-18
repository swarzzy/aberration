#pragma once
#include "Input.h"
#include "Shared.h"

namespace AB {

	struct InputMgr;
	struct WindowProperties;
	struct MemoryArena;
	
	typedef void(PlatformCloseCallback)(void);
	typedef void(PlatformResizeCallback)(uint32 width, uint32 height);

	WindowProperties* WindowAllocate(MemoryArena* arena);
	void WindowInit(WindowProperties* window, const char* title,
					uint32 width, uint32 height);
	bool WindowIsOpen(WindowProperties* window);
	void WindowPollEvents(WindowProperties* window);
	void WindowSwapBuffers(WindowProperties* window);
	void WindowEnableVSync(WindowProperties* window, bool32 enable);
	void WindowGetSize(WindowProperties* window, uint32* width, uint32* height);
	bool32 WindowActive(WindowProperties* window);
	void WindowShowCursor(WindowProperties* window, bool32 show);
	void WindowSetInputStatePtr(WindowProperties* window, InputState* appPtr);

	
	void WindowDestroy(WindowProperties* window);
	void WindowClose(WindowProperties* window);
	void WindowSetMousePosition(WindowProperties* window, uint32 x, uint32 y);

	void WindowSetCloseCallback(WindowProperties* window, PlatformCloseCallback* func);
	void WindowSetResizeCallback(WindowProperties* window, PlatformResizeCallback* func);
};

