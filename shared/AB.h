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

union Color {
	uint32 hex;
	struct {
		uint8 r;
		uint8 g;
		uint8 b;
		uint8 a;
	};
};

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

#if 0
#if defined(AB_BUILD_DLL)
#	if defined(AB_PLATFORM_WINDOWS)
#		define AB_API __declspec(dllexport)
#	else
#		define AB_API
#	endif
#else
#	if defined(AB_PLATFORM_WINDOWS)
#		define AB_API __declspec(dllimport)
#	else
#		define AB_API
#	endif
#endif
#else
#define AB_API
#endif

#if defined(AB_PLATFORM_WINDOWS)
#define GAME_CODE_ENTRY __declspec(dllexport)
#elif defined(AB_PLATFORM_LINUX)
#define GAME_CODE_ENTRY
#else
#error Unsupported OS
#endif

#define AB_BIT(shift) (1 << shift)

#if defined(AB_PLATFORM_WINDOWS)
// TODO: Make this work
#define AB_DEBUG_BREAK() __debugbreak()
#elif defined(AB_PLATFORM_LINUX)
#define AB_DEBUG_BREAK() __builtin_debugtrap()
#endif

inline uint32 SafeCastU64U32(uint64 val) {
	// TODO: Logging
	if (val > 0xffffffff) {
		AB_DEBUG_BREAK();
		return 0;
	} else {
		return (uint32)val;
	}
}

inline int32 SafeCastIntI32(int val) {
	if (sizeof(int) <= sizeof(int32)) {
		return (int32)val;
	} else if (val <= 0xffffffff) {
		return (int32)val;
	} else {
		AB_DEBUG_BREAK();
		return 0;
	}
}

inline uint32 SafeCastUintU32(unsigned int val) {
	if (sizeof(unsigned int) <= sizeof(uint32)) {
		return (uint32)val;
	} else if (val <= 0xffffffff) {
		return (uint32)val;
	} else {
		AB_DEBUG_BREAK();
		return 0;
	}
}

inline int SafeCastI32Int(int32 val) {
	if (sizeof(int) >= sizeof(uint32)) {
		return (int)val;
	}
	// TODO: Not aborting if int actually can hold the value 
	AB_DEBUG_BREAK();
	return 0;
}

inline uint8 SafeCastUptrU8(uintptr uptr) {
	if (uptr <= 255) {
		return (uint8)uptr;
	} else {
		AB_DEBUG_BREAK();
		return 0;
	}
}

inline uint32 SafeCastUptrU32(uintptr uptr) {
	if (uptr <= 0xffffffff) {
		return (uint32)uptr;
	} else {
		AB_DEBUG_BREAK();
		return 0;		
	}
}

inline u32 TruncateF32U32(f32 value)
{
	return (u32)value;
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
