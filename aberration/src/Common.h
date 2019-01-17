#pragma once

// Dynamic lib export/import macro
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

// This macro disallows copy and move constructors and assign operators
#define AB_DISALLOW_COPY_AND_MOVE(TypeName) \
	TypeName(const TypeName& other) = delete;\
	TypeName(TypeName&& TypeName) = delete;\
	TypeName& operator=(const TypeName& other) = delete;\
	TypeName& operator=(TypeName&& other) = delete;

#define AB_DEPRECATED [[deprecated]]

#define AB_BIT(shift) (1 << shift)