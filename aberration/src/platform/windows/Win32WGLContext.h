#pragma once

extern "C" {

#include <windows.h>
#include <gl/GL.h>

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

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

	typedef const char* APIENTRY _procDef_wglGetExtensionsStringARB(HDC);
	typedef BOOL		APIENTRY _procDef_wglChoosePixelFormatARB(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
	typedef HGLRC		APIENTRY _procDef_wglCreateContextAttribsARB(HDC, HGLRC, const int*);

	struct WGLProcs {
		_procDef_wglGetExtensionsStringARB*		ptr_wglGetExtensionsStringARB;
		_procDef_wglChoosePixelFormatARB*		ptr_wglChoosePixelFormatARB;
		_procDef_wglCreateContextAttribsARB*	ptr_wglCreateContextAttribsARB;
	};

	extern struct WGLProcs _wglProcs;

#define wglGetExtensionsStringARB		_wglProcs.ptr_wglGetExtensionsStringARB
#define wglChoosePixelFormatARB			_wglProcs.ptr_wglChoosePixelFormatARB
#define wglCreateContextAttribsARB		_wglProcs.ptr_wglCreateContextAttribsARB
}

namespace AB {
	unsigned int Win32LoadWGLProcs(HDC windowDC);
}