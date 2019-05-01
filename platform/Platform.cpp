#define HYPERMATH_IMPL 
#include <hypermath.h>
#include "Application.cpp"
#include "Common.cpp"
#include "PlatformMemory.cpp"
#include "PlatformOpenGL.cpp"

// NOTE: Shared
#include "Log.cpp"

#if defined(AB_PLATFORM_WINDOWS)
#include "windows/Win32Common.cpp"
#include "windows/Win32Window.cpp"
#include "windows/Win32WGL.cpp"
#include "windows/Win32CodeLoader.cpp"
#elif defined(AB_PLATFORM_LINUX)
#include "unix/UnixCommon.cpp"
#include "unix/X11Window.cpp"
#endif
