#include "InputManager.cpp"
#include "Memory.cpp"
#include "Common.cpp"

#include "API/GraphicsAPI.cpp"
#include "API/OpenGL/OpenGL.cpp"

#if defined(AB_PLATFORM_WINDOWS)
#include "windows/Win32Common.cpp"
#include "windows/Win32Window.cpp"
#include "windows/Win32WGL.cpp"
#elif defined(AB_PLATFORM_LINUX)
#include "unix/UnixCommon.cpp"
#include "unix/X11Window.cpp"
#endif
