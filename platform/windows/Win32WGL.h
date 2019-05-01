#pragma once

extern "C" {

#include <windows.h>
//#include <gl/GL.h>
#define GL_TRUE                           1
#define GL_FALSE                          0

#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_NO_ACCELERATION_ARB           0x2025

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB       0x2093
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// WGL_ARB_multisample
//#define	WGL_SAMPLE_BUFFERS_ARB            0x2041
//#define	WGL_SAMPLES_ARB                   0x2042
//#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB  0x20A9

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

	typedef const char* APIENTRY _proc_wglGetExtensionsStringARB(HDC);
	typedef BOOL		APIENTRY _proc_wglChoosePixelFormatARB(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
	typedef HGLRC		APIENTRY _proc_wglCreateContextAttribsARB(HDC, HGLRC, const int*);
	typedef BOOL		APIENTRY _proc_wglSwapIntervalEXT(int interval);
	typedef int			APIENTRY _proc_wglGetSwapIntervalEXT(void);

#define wglGetExtensionsStringARB		Win32::_wglProcs.ptr_wglGetExtensionsStringARB
#define wglChoosePixelFormatARB			Win32::_wglProcs.ptr_wglChoosePixelFormatARB
#define wglCreateContextAttribsARB		Win32::_wglProcs.ptr_wglCreateContextAttribsARB
#define wglSwapIntervalEXT				Win32::_wglProcs.ptr_wglSwapIntervalEXT
#define wglGetSwapIntervalEXT			Win32::_wglProcs.ptr_wglGetSwapIntervalEXT
}

namespace AB::Win32 {
	struct WGLProcs {
		_proc_wglGetExtensionsStringARB*	ptr_wglGetExtensionsStringARB;
		_proc_wglChoosePixelFormatARB*		ptr_wglChoosePixelFormatARB;
		_proc_wglCreateContextAttribsARB*	ptr_wglCreateContextAttribsARB;
		_proc_wglSwapIntervalEXT*			ptr_wglSwapIntervalEXT;
		_proc_wglGetSwapIntervalEXT*		ptr_wglGetSwapIntervalEXT;
	};

	extern struct WGLProcs _wglProcs;

	unsigned int LoadWGLFunctions(HDC windowDC);
}
