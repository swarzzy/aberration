#pragma once
#include "AB.h"
#include "Input.h"
#include "OpenGL.h"

namespace AB
{
	struct MemoryArena;
	
	struct DateTime
	{
		u16 year;
		u16 month;
		u16 dayOfWeek;
		u16 day;
		u16 hour;
		u16 minute;
		u16 seconds;
		u16 milliseconds;
	};
	
	enum class ConsoleColor : u16
	{
		Black = 0, DarkBlue, DarkGreen, DarkCyan, DarkRed, DarkPurple, DarkYellow,
		DarkWhite, Gray, Blue, Green, Cyan, Red, Purple, Yellow, White, Default
	};

	constexpr u16 DATETIME_STRING_SIZE = 9; // hh:mm:ss\0

	typedef void(PlatformMouseCallbackFn)(void* inputManager,
										  u32 xPos, u32 yPos,
										  u32 winW, u32 winH);

	typedef void(PlatformMouseButtonCallbackFn)(void* inputManager,
												MouseButton button, b32 state);

	typedef void(PlatformKeyCallbackFn)(void* inputManager,
										KeyboardKey key, b32 state,
										u16 sysRepeatCount);

	typedef void(PlatformFocusCallbackFn)(void* inputManager,
										  b32 focus);

	typedef void(PlatformMouseLeaveCallbackFn)(void* inputManager,
											   b32 inClient);

	typedef void(PlatformMouseScrollCallbackFn)(void* inputManager,
												i32 offset);

	struct PlatformInputCallbacks
	{
		PlatformMouseCallbackFn* PlatformMouseCallback;
		PlatformMouseButtonCallbackFn* PlatformMouseButtonCallback;
		PlatformKeyCallbackFn* PlatformKeyCallback;
		PlatformFocusCallbackFn* PlatformFocusCallback;
		PlatformMouseLeaveCallbackFn* PlatformMouseLeaveCallback;
		PlatformMouseScrollCallbackFn* PlatformMouseScrollCallback;
	};

	typedef void(RegisterInputManagerFn)(void* inputMnanager,
										 PlatformInputCallbacks* callbacks);
	typedef void(WindowGetSizeFn)(u32* w, u32* h);
	typedef void(WindowSetMousePositionFn)(u32 x, u32 y);
	typedef b32(WindowActiveFn)(void);
	typedef i32(ConsolePrintFn)(const void* data, u32 count);
	typedef i32(ConsoleSetColorFn)(ConsoleColor textColor, ConsoleColor backColor);
	typedef void*(DebugReadFilePermanentFn)(MemoryArena* memory,
											const char* filename,
											u32* bytesRead);
	typedef u32(DebugGetFileSizeFn)(const char* filename);
	typedef u32(DebugReadFileFn)(void* buffer, u32 bufferSize,
								 const char* filename);
	typedef u32(DebugReadTextFileFn)(void* buffer, u32 bufferSize,
								 const char* filename);

	struct PlatformFuncTable
	{
		RegisterInputManagerFn* RegisterInputManager;
		WindowGetSizeFn* WindowGetSize;
		WindowSetMousePositionFn* PlatformSetCorsorPosition;
		WindowActiveFn* WindowActive;
		ConsolePrintFn* ConsolePrint;
		ConsoleSetColorFn* ConsoleSetColor;
		DebugReadFilePermanentFn* DebugReadFilePermanent;
		DebugGetFileSizeFn* DebugGetFileSize;
		DebugReadFileFn* DebugReadFile;
		DebugReadTextFileFn* DebugReadTextFile;
	};

	struct PlatformState
	{
		PlatformFuncTable functions;
		GLFuncTable* gl;
		i64 runningTime;
		i64 frameTime;
		i64 fps;
		i64 ups;
		f32 deltaTime;
		u32 windowWidth;
		u32 windowHeight;
		b32 windowActive;
		DateTime localTime;
	};
}
