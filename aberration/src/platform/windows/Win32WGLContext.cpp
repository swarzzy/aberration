#include "Win32WGLContext.h"
// TODO: get rid of strstr()
#include <cstring> // memset // strstr

struct WGLProcs _wglProcs;

namespace AB {

	unsigned int  Win32LoadWGLProcs(HDC windowDC) {
		memset(&_wglProcs, 0, sizeof(_wglProcs));
		_wglProcs.ptr_wglGetExtensionsStringARB = reinterpret_cast<_procDef_wglGetExtensionsStringARB*>(wglGetProcAddress("wglGetExtensionsStringARB"));
		if (!_wglProcs.ptr_wglGetExtensionsStringARB)
			return 0;

		const char* extensions = wglGetExtensionsStringARB(windowDC);
		if (!std::strstr(extensions, "WGL_ARB_pixel_format"))
			return 0;
		if (!std::strstr(extensions, "WGL_ARB_create_context_profile"))
			return 0;

		_wglProcs.ptr_wglChoosePixelFormatARB = reinterpret_cast<_procDef_wglChoosePixelFormatARB*>(wglGetProcAddress("wglChoosePixelFormatARB"));
		if (!_wglProcs.ptr_wglChoosePixelFormatARB)
			return 0;

		_wglProcs.ptr_wglCreateContextAttribsARB = reinterpret_cast<_procDef_wglCreateContextAttribsARB*>(wglGetProcAddress("wglCreateContextAttribsARB"));
		if (!_wglProcs.ptr_wglCreateContextAttribsARB)
			return 0;

		return 1;
	}
}
