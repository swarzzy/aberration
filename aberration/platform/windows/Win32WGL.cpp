#include "Win32WGL.h"
// TODO: get rid of strstr()
#include <cstring> // memset // strstr


namespace AB::Win32 {
	struct WGLProcs _wglProcs;

	unsigned int  LoadWGLFunctions(HDC windowDC) {
		memset(&_wglProcs, 0, sizeof(_wglProcs));
		_wglProcs.ptr_wglGetExtensionsStringARB = reinterpret_cast<_proc_wglGetExtensionsStringARB*>(wglGetProcAddress("wglGetExtensionsStringARB"));
		if (!_wglProcs.ptr_wglGetExtensionsStringARB)
			return 0;

		const char* extensions = wglGetExtensionsStringARB(windowDC);
		if (!strstr(extensions, "WGL_ARB_pixel_format"))
			return 0;
		if (!strstr(extensions, "WGL_ARB_create_context_profile"))
			return 0;
		if (!strstr(extensions, "EXT_swap_control"))
			return 0;
		//if (!strstr(extensions, "WGL_ARB_multisample"))
		//return 0;
		//if (!strstr(extensions, "WGL_ARB_framebuffer_sRGB"))
		//return 0;

		_wglProcs.ptr_wglChoosePixelFormatARB = reinterpret_cast<_proc_wglChoosePixelFormatARB*>(wglGetProcAddress("wglChoosePixelFormatARB"));
		if (!_wglProcs.ptr_wglChoosePixelFormatARB)
			return 0;

		_wglProcs.ptr_wglCreateContextAttribsARB = reinterpret_cast<_proc_wglCreateContextAttribsARB*>(wglGetProcAddress("wglCreateContextAttribsARB"));
		if (!_wglProcs.ptr_wglCreateContextAttribsARB)
			return 0;

		_wglProcs.ptr_wglSwapIntervalEXT = reinterpret_cast<_proc_wglSwapIntervalEXT*>(wglGetProcAddress("wglSwapIntervalEXT"));
		if (!_wglProcs.ptr_wglCreateContextAttribsARB)
			return 0;

		_wglProcs.ptr_wglGetSwapIntervalEXT = reinterpret_cast<_proc_wglGetSwapIntervalEXT*>(wglGetProcAddress("wglGetSwapIntervalEXT"));
		if (!_wglProcs.ptr_wglCreateContextAttribsARB)
			return 0;

		return 1;
	}
}
