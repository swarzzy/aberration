#pragma once

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