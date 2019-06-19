#pragma once
#include <cstdint>

typedef int8_t				int8;	
typedef int16_t				int16;	
typedef int32_t				int32;	
typedef int64_t				int64;	

typedef uint8_t				uint8;	
typedef uint16_t			uint16;	
typedef uint32_t			uint32;	
typedef uint64_t			uint64;

typedef uintptr_t			uintptr;

#undef byte // Defined as unsigned char in rpcndr.h
typedef char				byte;
typedef uint32				bool32;
typedef unsigned char		uchar;

typedef float				float32;
typedef double				float64;

typedef uint32				color32;
typedef byte				bool8;

// new typedefs
typedef int8_t				i8;	
typedef int16_t				i16;	
typedef int32_t				i32;	
typedef int64_t				i64;	

typedef uint8_t				u8;	
typedef uint16_t			u16;	
typedef uint32_t			u32;	
typedef uint64_t			u64;

typedef uintptr_t			uptr;

#undef byte // Defined as unsigned char in rpcndr.h
typedef uint32				b32;

typedef float				f32;
typedef double				f64;

typedef byte				b8;

#define AB_API

#define AB_UINT32_MAX  	((u32)(0xffffffff))
#define AB_INT32_MIN 	(-(i32)(2147483648))
#define AB_INT32_MAX 	((i32)(2147483647))

#define restrict __restrict

#if defined(AB_PLATFORM_WINDOWS)
#define GAME_CODE_ENTRY __declspec(dllexport)
#elif defined(AB_PLATFORM_LINUX)
#define GAME_CODE_ENTRY
#else
#error Unsupported OS
#endif

#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))

#if defined(AB_PLATFORM_WINDOWS)
#define AB_DEBUG_BREAK() __debugbreak()
#elif defined(AB_PLATFORM_LINUX)
#define AB_DEBUG_BREAK() __builtin_debugtrap()
#endif

#include "Input.h"
#include "OpenGL.h"

namespace AB
{
	inline u32 SafeCastU64U32(u64 val)
	{
		// TODO: Logging
		if (val > 0xffffffff)
		{
			AB_DEBUG_BREAK();
			return 0;
		}
		else
		{
			return (u32)val;
		}
	}

	inline i32 SafeCastIntI32(int val)
	{
		if (sizeof(int) <= sizeof(i32))
		{
			return (i32)val;
		}
		else if (val <= 0xffffffff)
		{
			return (i32)val;
		}
		else
		{
			AB_DEBUG_BREAK();
			return 0;
		}
	}

	inline u32 SafeCastUintU32(unsigned int val)
	{
		if (sizeof(unsigned int) <= sizeof(u32))
		{
			return (u32)val;
		}
		else if (val <= 0xffffffff)
		{
			return (u32)val;
		}
		else
		{
			AB_DEBUG_BREAK();
			return 0;
		}
	}

	inline int SafeCastI32Int(i32 val)
	{
		if (sizeof(int) >= sizeof(u32))
		{
			return (int)val;
		}
		// TODO: Not aborting if int actually can hold the value 
		AB_DEBUG_BREAK();
		return 0;
	}

	inline u32 SafeCastUptrU32(uptr uptr)
	{
		if (uptr <= 0xffffffff)
		{
			return (uint32)uptr;
		}
		else
		{
			AB_DEBUG_BREAK();
			return 0;		
		}
	}

	inline u32 TruncF32U32(f32 value)
	{
		return (u32)value;
	}

	inline i32 TruncF32I32(f32 value)
	{
		return (i32)value;
	}


	inline i32 SafeCastU32I32(u32 val)
	{
		if (val <= 0x7fffffff)
		{
			return (i32)val;
		}
		else
		{
			AB_DEBUG_BREAK();
			return 0;				
		}
	}

	inline u32 AbsI32U32(i32 value)
	{
		return value >= 0 ? value : -value;
	}

	inline u32 AbsU32U32(i32 value)
	{
		return value >= 0 ? value : -value;
	}

	inline u32 TruncateI32U32(i32 value)
	{
		return value >= 0 ? value : 0;
	}

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

	//typedef void(WindowGetSizeFn)(u32* w, u32* h);
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

	typedef void(GetLocalTimeFn)(DateTime* datetime);

	enum GameUpdateAndRenderReason
	{
		GUR_REASON_INIT, GUR_REASON_RELOAD, GUR_REASON_UPDATE, GUR_REASON_RENDER
	};

		
	struct PlatformFuncTable
	{
		//WindowGetSizeFn* WindowGetSize;
		WindowSetMousePositionFn* PlatformSetCursorPosition;
		WindowActiveFn* WindowActive;
		ConsolePrintFn* ConsolePrint;
		ConsoleSetColorFn* ConsoleSetColor;
		DebugReadFilePermanentFn* DebugReadFilePermanent;
		DebugGetFileSizeFn* DebugGetFileSize;
		DebugReadFileFn* DebugReadFile;
		DebugReadTextFileFn* DebugReadTextFile;
		GetLocalTimeFn* GetLocalTime;
	};

	struct KeyState
	{
		// TODO: Shpuld they be b32?
		b32 pressedNow;
		b32 wasPressed;
	};

	struct MButtonState
	{
		b32 pressedNow;
		b32 wasPressed;
	};
	
	struct InputState
	{
		// NOTE: Is b8 good choise?
		KeyState keys[KEYBOARD_KEYS_COUNT];
		MButtonState mouseButtons[MOUSE_BUTTONS_COUNT];
		b32 mouseInWindow;
		b32 activeApp;
		// NOTE: All mouse position values are normalized
		f32 mouseX;
		f32 mouseY;
		f32 mouseFrameOffsetX;
		f32 mouseFrameOffsetY;
		// NOTE: Not normalized
		i32 scrollOffset;
		//i32 prevFrameScrollOffset;
		i32 scrollFrameOffset;
	};

	struct PlatformState
	{
		PlatformFuncTable functions;
		GLFuncTable* gl;
		InputState input;
		i64 runningTime;
		i64 frameTime;
		i64 fps;
		i64 ups;
		f32 gameSpeed;
		f32 absDeltaTime;
		f32 gameDeltaTime;
		u32 windowWidth;
		u32 windowHeight;
		DateTime localTime;
	};
}
