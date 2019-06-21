#pragma once
#include "Shared.h"
#include <Windows.h>
#include "Win32CodeLoader.h"
#include "Win32Common.h"

extern "C"
{
	typedef const char*(APIENTRY wglGetExtensionsStringARBFn)(HDC);
	typedef BOOL (APIENTRY wglChoosePixelFormatARBFn)(HDC, const int*,
														const FLOAT*, UINT, int*,
														UINT*); 
	typedef HGLRC (APIENTRY wglCreateContextAttribsARBFn)(HDC, HGLRC,
															 const int*);
	typedef BOOL (APIENTRY wglSwapIntervalEXTFn)(int interval);
	typedef int	(APIENTRY wglGetSwapIntervalEXTFn)(void);
}

namespace AB
{

	static const char* WINDOW_CLASS_NAME = "Aberration Engine Win32";

	static const uint32 OPENGL_MAJOR_VERSION = 4;
	static const uint32 OPENGL_MINOR_VERSION = 5;

	static const uint32 WINDOW_TITLE_SIZE = 32;

	struct GameCode;

	struct Application
	{
		MemoryArena* systemMemory;
		MemoryArena* gameMemory;
		void* gameStaticStorage;
		PlatformState state;
		
		char windowTitle[32];
		bool32 running;
		bool32 fullscreen;
		HWND win32WindowHandle;
		HDC win32WindowDC;
		HGLRC OpenGLRC;
		WINDOWPLACEMENT wpPrev;

		TRACKMOUSEEVENT Win32MouseTrackEvent;

		wglGetExtensionsStringARBFn* wglGetExtensionsStringARB;
		wglChoosePixelFormatARBFn* wglChoosePixelFormatARB;
		wglCreateContextAttribsARBFn* wglCreateContextAttribsARB;
		wglSwapIntervalEXTFn* wglSwapIntervalEXT;
		wglGetSwapIntervalEXTFn* wglGetSwapIntervalEXT;
		uint8 keyTable[KEYBOARD_KEYS_COUNT];

		GameUpdateAndRenderFn* GameUpdateAndRender;
		u64 libLastChangeTime;
		char libFullPath[MAX_GAME_LIB_PATH];
		char libDir[MAX_GAME_LIB_PATH];
		HMODULE libHandle;
	};

	static const i64 UPDATE_INTERVAL = 16000;
	static const i64 SECOND_INTERVAL = 1000000;

	MemoryArena* AllocateArena(uptr size);
	Application* AppCreate(MemoryArena* sysMemory);
	void AppRun(Application* app);

	struct WindowProperties;
	struct MemoryArena;
	
	void
		WindowPollEvents(Application* app);
	void
		WindowToggleFullscreen(Application* app, bool enable);
	void
		WindowShowCursor(Application* app, bool32 show);
	void
		WindowSetMousePosition(Application* app, uint32 x, uint32 y);

	static LRESULT CALLBACK
		Win32WindowCallback(HWND windowHandle, UINT message,
							WPARAM wParam, LPARAM lParam);
	static void
		Win32InitKeyTable(uint8* keytable);
	
	static uint8
		Win32KeyConvertToABKeycode(Application* app, uint64 Win32Key);
	
	static void
		Win32Initialize(Application* app);

	unsigned int
		WGLLoadFunctions(Application* app, HDC windowDC);

};

