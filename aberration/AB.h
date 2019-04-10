#pragma once
#if defined(_MSC_VER) 
// Because exceptions are disabled
#pragma warning(disable : 4530)
#endif

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
typedef uint8				byte;
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

#define AB_DISALLOW_COPY_AND_MOVE(TypeName) \
	TypeName(const TypeName& other) = delete;\
	TypeName(TypeName&& TypeName) = delete;\
	TypeName& operator=(const TypeName& other) = delete;\
	TypeName& operator=(TypeName&& other) = delete;

#define AB_BIT(shift) (1 << shift)

#if defined(AB_PLATFORM_WINDOWS)
// TODO: Make this work
#define AB_DEBUG_BREAK()  *(byte *)(0)//__debugbreak()
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
