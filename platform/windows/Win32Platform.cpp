#include "Win32Platform.h"

#define HYPERMATH_IMPL 
#include <hypermath.h>

#include "Log.h"

#include "Memory.h"
#include <hypermath.h>

#include "OpenGL.h"
#include "../OpenGLLoader.h"

#include <cstdlib>

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

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

// FORWARD DECLARATIONS

// Macros from windowsx.h
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam) ((u16)(lParam & 0xffff >> 2))

namespace AB
{
	
	inline static void
	ProcessMButtonEvent(InputState* input, MouseButton button, b32 state) 
	{
		input->mouseButtons[button].wasPressed =
			input->mouseButtons[button].pressedNow;
		input->mouseButtons[button].pressedNow = state;
	}

	// NOTE: Based on Raymond Chen example
	// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
	void WindowToggleFullscreen(Application* app, bool enable)
	{
		DWORD style = GetWindowLong(app->win32WindowHandle, GWL_STYLE);
		if ((style & WS_OVERLAPPEDWINDOW) && enable)
		{
			app->fullscreen = true;
			MONITORINFO mInfo = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(app->win32WindowHandle, &app->wpPrev) &&
				GetMonitorInfo(MonitorFromWindow(app->win32WindowHandle,
												 MONITOR_DEFAULTTOPRIMARY), &mInfo))
			{
				SetWindowLong(app->win32WindowHandle, GWL_STYLE,
							  style & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(app->win32WindowHandle, HWND_TOP,
							 mInfo.rcMonitor.left, mInfo.rcMonitor.top,
							 mInfo.rcMonitor.right - mInfo.rcMonitor.left,
							 mInfo.rcMonitor.bottom - mInfo.rcMonitor.top,
							 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			}
		}
		else if (!enable)
		{
			app->fullscreen = false;
			SetWindowLong(app->win32WindowHandle, GWL_STYLE,
						  style | WS_OVERLAPPEDWINDOW);
			SetWindowPlacement(app->win32WindowHandle, &app->wpPrev);
			SetWindowPos(app->win32WindowHandle, nullptr, 0, 0, 0, 0,
						 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}

	void WindowPollEvents(Application* app)
	{
	   	MSG message;
		BOOL result;
		while (app->running &&
			   (result = PeekMessage(&message, app->win32WindowHandle,
									 0, 0, PM_REMOVE)) != 0)
		{
			if (result == -1)
			{
				AB_CORE_FATAL("Window recieve error message.");
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
	}

	void WindowSetMousePosition(Application* app, uint32 x, uint32 y)
	{
		uint32 yFlipped = 0;
		if (y < app->state.windowHeight)
		{
			yFlipped = app->state.windowHeight - y;
			POINT pt = { (LONG)x, (LONG)yFlipped };
			if (ClientToScreen(app->win32WindowHandle, &pt))
			{
				SetCursorPos(pt.x, pt.y);
			}
		}
	}

	void WindowShowCursor(Application* app, bool32 show)
	{
		// TODO: Make this work (SetCursor())
		//::ShowCursor(show ? TRUE : FALSE);
	}

	void Win32Initialize(Application* app)
	{
		app->wpPrev = {sizeof(WINDOWPLACEMENT)};
		auto instance = GetModuleHandle(0);

		WNDCLASS windowClass = {};
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = Win32WindowCallback;
		windowClass.hInstance = instance;
		windowClass.lpszClassName = WINDOW_CLASS_NAME;
		windowClass.hCursor = LoadCursor(0 ,IDC_ARROW);

		auto RCresult = RegisterClass(&windowClass);
		AB_CORE_ASSERT(RCresult, "Failed to create window.");

		HWND fakeWindow = CreateWindowEx(NULL, windowClass.lpszClassName,
										 "AB Dummy window", WS_OVERLAPPEDWINDOW,
										 CW_USEDEFAULT,  CW_USEDEFAULT,	1, 1,
										 NULL, NULL, instance, NULL);

		HDC fakeWindowDC = GetDC(fakeWindow);

		PIXELFORMATDESCRIPTOR fakeDesiredPixelFormat = {};
		fakeDesiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		fakeDesiredPixelFormat.nVersion = 1;
		fakeDesiredPixelFormat.dwFlags =
			PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		fakeDesiredPixelFormat.cColorBits = 32;
		fakeDesiredPixelFormat.cAlphaBits = 8;
		fakeDesiredPixelFormat.cDepthBits = 24;
		fakeDesiredPixelFormat.cStencilBits = 8;

		auto fakeActualPFIndex = ChoosePixelFormat(fakeWindowDC,
												   &fakeDesiredPixelFormat);

		PIXELFORMATDESCRIPTOR fakeActualPixelFormat = {};
		DescribePixelFormat(fakeWindowDC, fakeActualPFIndex,
							sizeof(PIXELFORMATDESCRIPTOR), &fakeActualPixelFormat);
		SetPixelFormat(fakeWindowDC, fakeActualPFIndex, &fakeActualPixelFormat);

		HGLRC fakeGLRC = wglCreateContext(fakeWindowDC);
		auto resultMC = wglMakeCurrent(fakeWindowDC, fakeGLRC);
		AB_CORE_ASSERT(resultMC, "Failed to create OpenGL context.");
		// TODO: Should it release dc?
		//ReleaseDC(windowHandle, windowDC);

		auto wglLoadProcsResult = WGLLoadFunctions(app, fakeWindowDC);
		AB_CORE_ASSERT(wglLoadProcsResult, "Failed to load WGL extensions");

		// ACTUAL WINDOW

		RECT actualSize = {};
		actualSize.top = 0;
		actualSize.left = 0;
		actualSize.right = app->state.windowWidth;
		actualSize.bottom = app->state.windowHeight;

		AdjustWindowRectEx(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						   NULL, NULL);

		int32 width = AbsI32(actualSize.left) + AbsI32(actualSize.right);
		int32 height = AbsI32(actualSize.top) + AbsI32(actualSize.bottom);
		HWND actualWindowHandle = CreateWindowEx(NULL, windowClass.lpszClassName,
												 app->windowTitle,
												 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
												 CW_USEDEFAULT, CW_USEDEFAULT,
												 app->state.windowWidth,
												 app->state.windowHeight,
												 NULL, NULL, instance, app);

		AB_CORE_ASSERT(actualWindowHandle, "Failed to create window.");

		HDC actualWindowDC = GetDC(actualWindowHandle);

		// ^^^^ ACTUAL WINDOW

		int attribList[] =
	   {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			//WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, 
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			//WGL_DEPTH_BITS_ARB, 24,
			//WGL_STENCIL_BITS_ARB, 8,
			//SafeCastI32Int(multisampling), 1,
			//WGL_SAMPLES_ARB, SafeCastI32Int(window->samples),
			0
		};

		int actualPixelFormatID = 0;
		UINT numFormats = 0;
		// Here was fake DC
		auto resultCPF = app->wglChoosePixelFormatARB(actualWindowDC,
													  attribList, nullptr,
													  1, &actualPixelFormatID,
													  &numFormats);
		AB_CORE_ASSERT(resultCPF, "Failed to initialize OpenGL extended context.");

		PIXELFORMATDESCRIPTOR actualPixelFormat = {};
		auto resultDPF = DescribePixelFormat(actualWindowDC,
											 actualPixelFormatID,
											 sizeof(PIXELFORMATDESCRIPTOR),
											 &actualPixelFormat);
		AB_CORE_ASSERT(resultDPF, "Failed to initialize OpenGL extended context.");
		SetPixelFormat(actualWindowDC, actualPixelFormatID, &actualPixelFormat);

		int contextAttribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR_VERSION,
				WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR_VERSION,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};

		HGLRC actualGLRC = app->wglCreateContextAttribsARB(actualWindowDC,
														   0, contextAttribs);
		AB_CORE_ASSERT(actualGLRC, "Failed to initialize OpenGL extended context");

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(fakeGLRC);
		ReleaseDC(fakeWindow, fakeWindowDC);
		DestroyWindow(fakeWindow);

		resultMC = wglMakeCurrent(actualWindowDC, actualGLRC);
		AB_CORE_ASSERT(resultMC, "Failed to initialize OpenGL extended context");

		app->win32WindowHandle = actualWindowHandle;
		app->win32WindowDC = actualWindowDC;
		app->OpenGLRC = actualGLRC;

		app->Win32MouseTrackEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		app->Win32MouseTrackEvent.dwFlags = TME_LEAVE;
		app->Win32MouseTrackEvent.dwHoverTime = HOVER_DEFAULT;
		app->Win32MouseTrackEvent.hwndTrack = app->win32WindowHandle;
		TrackMouseEvent(&app->Win32MouseTrackEvent);

		Win32InitKeyTable(app->keyTable);

		SetFocus(app->win32WindowHandle);
	}

	static LRESULT CALLBACK
	Win32WindowCallback(HWND windowHandle, UINT message,
						WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;

		if (message == WM_CREATE)
		{
			CREATESTRUCT* data = (CREATESTRUCT*)lParam;
			Application* app = (Application*)data->lpCreateParams;
			if (app)
			{
				SetWindowLongPtr(windowHandle,
								 GWLP_USERDATA, (LONG_PTR)app);
				app->running = true;
			}
			return result;
		}

		auto ptr = GetWindowLongPtr(windowHandle, GWLP_USERDATA);

		if (ptr)
		{
			Application* app = (Application*)ptr;

			switch (message)
			{
			case WM_SIZE:
			{
				app->state.windowWidth = LOWORD(lParam);
				app->state.windowHeight = HIWORD(lParam);
			} break;

			case WM_DESTROY:
			{
				PostQuitMessage(0);
			} break;

			case WM_CLOSE:
			{
				app->running = false;
				ShowWindow(app->win32WindowHandle, SW_HIDE);
			} break;

			// NOTE: MOUSE INPUT

			case WM_MOUSEMOVE:
			{
				if (!app->state.input.mouseInWindow)
				{
					app->state.input.mouseInWindow = true;
					TrackMouseEvent(&app->Win32MouseTrackEvent);
				}
				int32 mousePositionX = GET_X_LPARAM(lParam);
				int32 mousePositionY = GET_Y_LPARAM(lParam);
				
				mousePositionY = app->state.windowHeight - mousePositionY;

				f32 normalizedMouseX = (f32)mousePositionX /
					(f32)app->state.windowWidth;
				f32 normalizedMouseY = (f32)mousePositionY /
					(f32)app->state.windowHeight;
				
				app->state.input.mouseFrameOffsetX =
					normalizedMouseX - app->state.input.mouseX;
				
				app->state.input.mouseFrameOffsetY =
					normalizedMouseY - app->state.input.mouseY;
				
				app->state.input.mouseX = normalizedMouseX;
				app->state.input.mouseY = normalizedMouseY;
				
			} break;

			case WM_LBUTTONDOWN:
			{
				ProcessMButtonEvent(&app->state.input, MBUTTON_LEFT, true);
			} break;

			case WM_LBUTTONUP:
			{
				ProcessMButtonEvent(&app->state.input, MBUTTON_LEFT, false);
			} break;

			case WM_RBUTTONDOWN:
			{
				ProcessMButtonEvent(&app->state.input, MBUTTON_RIGHT, true);
			} break;

			case WM_RBUTTONUP:
			{
				ProcessMButtonEvent(&app->state.input, MBUTTON_RIGHT, false);
			} break;

			case WM_MBUTTONDOWN:
			{
				ProcessMButtonEvent(&app->state.input, MBUTTON_MIDDLE, true);
			} break;

			case WM_MBUTTONUP:
			{
				ProcessMButtonEvent(&app->state.input, MBUTTON_MIDDLE, false);
			} break;

			case WM_XBUTTONDOWN:
			{
				auto state = HIWORD(wParam);
				if (state & XBUTTON1)
				{
					ProcessMButtonEvent(&app->state.input, MBUTTON_XBUTTON1, true);
				}
				else
				{
					ProcessMButtonEvent(&app->state.input, MBUTTON_XBUTTON2, true);
				}
			} break;

			case WM_XBUTTONUP:
			{
				auto state = HIWORD(wParam);
				if (state & XBUTTON1)
				{
					ProcessMButtonEvent(&app->state.input,MBUTTON_XBUTTON1, false);
				}
				else
				{
					ProcessMButtonEvent(&app->state.input,MBUTTON_XBUTTON2, false);
				}
			} break;

			case WM_MOUSELEAVE:
			{
				app->state.input.mouseInWindow = false;
			} break;

			case WM_MOUSEWHEEL:
			{
				i32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
				i32 numSteps = delta / WHEEL_DELTA;
				app->state.input.scrollOffset = numSteps;
				app->state.input.scrollFrameOffset = numSteps;
			} break;

			// ^^^^ MOUSE INPUT
			// KEYBOARD INPUT

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				// TODO: Repeat counts for now doesnt working on windows
				// TODO: Why they are not working and why is this TODO here?
				uint32 key = Win32KeyConvertToABKeycode(app, wParam);
				bool32 state = true;
				uint16 sys_repeat_count =  AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam);
				app->state.input.keys[key].wasPressed =
					app->state.input.keys[key].pressedNow;
				app->state.input.keys[key].pressedNow = state;
				// TODO: Temorary
				if (app->state.input.keys[KEY_L].pressedNow &&
					!app->state.input.keys[KEY_L].wasPressed)
				{
					WindowToggleFullscreen(app, !app->fullscreen);
				}
			} break;

			case WM_SYSKEYUP:
			case WM_KEYUP:
			{
				uint32 key = Win32KeyConvertToABKeycode(app, wParam);
				bool32 state = false;
				uint16 sys_repeat_count = 0;
				app->state.input.keys[key].wasPressed =
					app->state.input.keys[key].pressedNow;
				app->state.input.keys[key].pressedNow = state;
			} break;

			case WM_CHAR:
			{
				u32 textBufferCount = app->state.input.textBufferCount;
				char* textBuffer =
					app->state.input.textBuffer + textBufferCount;
				// NOTE: Reserve last character because wcstombs
				// null terminates strings
				u32 textBufferFree =
					PLATFORM_TEXT_INPUT_BUFFER_SIZE - textBufferCount - 1;
				// TODO: wcstombs implementation
				if (textBufferFree)
				{
					size_t ret = wcstombs(textBuffer, (wchar_t*)(&wParam),
										  textBufferFree);
					if (ret != (size_t)(-1))
					{
						app->state.input.textBufferCount += (u32)ret;
					}
				}
			} break;
			// ^^^^ KEYBOARD INPUT

			case WM_ACTIVATEAPP:
			{
				if (wParam == TRUE)
				{
					app->state.input.activeApp = true;
				}
				else
				{
					app->state.input.activeApp = false;
				}
			} break;
			default:
			{
				result = DefWindowProc(windowHandle, message, wParam, lParam);
			} break;
			}
		}
		else
		{
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}
		return result;
	}
	
	static uint8
	Win32KeyConvertToABKeycode(Application* app, uint64 Win32Key)
	{
		if (Win32Key < KEYBOARD_KEYS_COUNT)
			return app->keyTable[Win32Key];
		return KEY_INVALIDKEY;
	}

	static void
	Win32InitKeyTable(uint8* keytable)
	{
		memset(keytable, KEY_INVALIDKEY, KEYBOARD_KEYS_COUNT);

		keytable[0x08] = KEY_BACKSPACE;
		keytable[0x09] = KEY_TAB;
		keytable[0x0c] = KEY_CLEAR;
		keytable[0x0d] = KEY_ENTER;
		keytable[0x10] = KEY_SHIFT;
		keytable[0x11] = KEY_CTRL;
		keytable[0x12] = KEY_ALT;
		keytable[0x13] = KEY_PAUSE;
		keytable[0x14] = KEY_CAPSLOCK;
		keytable[0x1b] = KEY_ESCAPE;
		keytable[0x20] = KEY_SPACE;
		keytable[0x21] = KEY_PAGEUP;
		keytable[0x22] = KEY_PAGEDOWN;
		keytable[0x23] = KEY_END;
		keytable[0x24] = KEY_HOME;
		keytable[0x25] = KEY_LEFT;
		keytable[0x26] = KEY_UP;
		keytable[0x27] = KEY_RIGHT;
		keytable[0x28] = KEY_DOWN;
		keytable[0x2c] = KEY_PRINTSCREEN;
		keytable[0x2d] = KEY_INSERT;
		keytable[0x2e] = KEY_DELETE;
		keytable[0x30] = KEY_KEY0;
		keytable[0x31] = KEY_KEY1;
		keytable[0x32] = KEY_KEY2;
		keytable[0x33] = KEY_KEY3;
		keytable[0x34] = KEY_KEY4;
		keytable[0x35] = KEY_KEY5;
		keytable[0x36] = KEY_KEY6;
		keytable[0x37] = KEY_KEY7;
		keytable[0x38] = KEY_KEY8;
		keytable[0x39] = KEY_KEY9;
		keytable[0x41] = KEY_A;
		keytable[0x42] = KEY_B;
		keytable[0x43] = KEY_C;
		keytable[0x44] = KEY_D;
		keytable[0x45] = KEY_E;
		keytable[0x46] = KEY_F;
		keytable[0x47] = KEY_G;
		keytable[0x48] = KEY_H;
		keytable[0x49] = KEY_I;
		keytable[0x4a] = KEY_J;
		keytable[0x4b] = KEY_K;
		keytable[0x4c] = KEY_L;
		keytable[0x4d] = KEY_M;
		keytable[0x4e] = KEY_N;
		keytable[0x4f] = KEY_O;
		keytable[0x50] = KEY_P;
		keytable[0x51] = KEY_Q;
		keytable[0x52] = KEY_R;
		keytable[0x53] = KEY_S;
		keytable[0x54] = KEY_T;
		keytable[0x55] = KEY_U;
		keytable[0x56] = KEY_V;
		keytable[0x57] = KEY_W;
		keytable[0x58] = KEY_X;
		keytable[0x59] = KEY_Y;
		keytable[0x5a] = KEY_Z;
		keytable[0x5b] = KEY_LEFTSUPER;
		keytable[0x5c] = KEY_RIGHTSUPER;
		keytable[0x60] = KEY_NUMPAD0;
		keytable[0x61] = KEY_NUMPAD1;
		keytable[0x62] = KEY_NUMPAD2;
		keytable[0x63] = KEY_NUMPAD3;
		keytable[0x64] = KEY_NUMPAD4;
		keytable[0x65] = KEY_NUMPAD5;
		keytable[0x66] = KEY_NUMPAD6;
		keytable[0x67] = KEY_NUMPAD7;
		keytable[0x68] = KEY_NUMPAD8;
		keytable[0x69] = KEY_NUMPAD9;
		keytable[0x6a] = KEY_NUMPADMULTIPLY;
		keytable[0x6b] = KEY_NUMPADADD;
		keytable[0x6d] = KEY_NUMPADSUBTRACT;
		keytable[0x6e] = KEY_NUMPADDECIMAL;
		keytable[0x6f] = KEY_NUMPADDIVIDE;
		keytable[0x70] = KEY_F1;
		keytable[0x71] = KEY_F2;
		keytable[0x72] = KEY_F3;
		keytable[0x73] = KEY_F4;
		keytable[0x74] = KEY_F5;
		keytable[0x75] = KEY_F6;
		keytable[0x76] = KEY_F7;
		keytable[0x77] = KEY_F8;
		keytable[0x78] = KEY_F9;
		keytable[0x79] = KEY_F10;
		keytable[0x7a] = KEY_F11;
		keytable[0x7b] = KEY_F12;
		keytable[0x7c] = KEY_F13;
		keytable[0x7d] = KEY_F14;
		keytable[0x7e] = KEY_F15;
		keytable[0x7f] = KEY_F16;
		keytable[0x80] = KEY_F17;
		keytable[0x81] = KEY_F18;
		keytable[0x82] = KEY_F19;
		keytable[0x83] = KEY_F20;
		keytable[0x84] = KEY_F21;
		keytable[0x85] = KEY_F22;
		keytable[0x86] = KEY_F23;
		keytable[0x87] = KEY_F24;
		keytable[0x90] = KEY_NUMLOCK;
		keytable[0x91] = KEY_SCROLLLOCK;
		keytable[0xa0] = KEY_LEFTSHIFT;
		keytable[0xa1] = KEY_RIGHTSHIFT;
		// Only Ctrl now works and processed by SYSKEY events
		//keytable[0xa2] = KeyboardKey::LeftCtrl;
		//keytable[0xa3] = KeyboardKey::RightCtrl;
		//keytable[0xa4] = KeyboardKey::LeftAlt; 0x11
		keytable[0xa5] = KEY_MENU;
		keytable[0xba] = KEY_SEMICOLON;
		keytable[0xbb] = KEY_EQUAL;
		keytable[0xbc] = KEY_COMMA;
		keytable[0xbd] = KEY_MINUS;
		keytable[0xbe] = KEY_PERIOD;
		keytable[0xbf] = KEY_SLASH;
		keytable[0xc0] = KEY_TILDE;
		keytable[0xdb] = KEY_LEFTBRACKET;
		keytable[0xdc] = KEY_BACKSLASH;
		keytable[0xdd] = KEY_RIGHTBRACKET;
		keytable[0xde] = KEY_APOSTROPHE;
	}

	unsigned int
	WGLLoadFunctions(Application* app, HDC windowDC)
	{
		app->wglGetExtensionsStringARB = (wglGetExtensionsStringARBFn*)wglGetProcAddress("wglGetExtensionsStringARB");
		if (!app->wglGetExtensionsStringARB)
			return 0;

		const char* extensions = app->wglGetExtensionsStringARB(windowDC);
		if (!strstr(extensions, "WGL_ARB_pixel_format"))
			return 0;
		if (!strstr(extensions, "WGL_ARB_create_context_profile"))
			return 0;
		if (!strstr(extensions, "EXT_swap_control"))
			return 0;

		app->wglChoosePixelFormatARB = (wglChoosePixelFormatARBFn*)wglGetProcAddress("wglChoosePixelFormatARB");
		if (!app->wglChoosePixelFormatARB)
			return 0;

		app->wglCreateContextAttribsARB = (wglCreateContextAttribsARBFn*)wglGetProcAddress("wglCreateContextAttribsARB");
		if (!app->wglCreateContextAttribsARB)
			return 0;

		app->wglSwapIntervalEXT = (wglSwapIntervalEXTFn*)wglGetProcAddress("wglSwapIntervalEXT");
		if (!app->wglCreateContextAttribsARB)
			return 0;

		app->wglGetSwapIntervalEXT = (wglGetSwapIntervalEXTFn*)wglGetProcAddress("wglGetSwapIntervalEXT");
		if (!app->wglCreateContextAttribsARB)
			return 0;

		return 1;
	}

	MemoryArena* AllocateArena(uptr size)
	{
		uptr headerSize = sizeof(MemoryArena);
		void* mem = VirtualAlloc(0, size + headerSize,
								 MEM_RESERVE | MEM_COMMIT,
								 PAGE_READWRITE);
		AB_CORE_ASSERT(mem, "Allocation failed");
		AB_CORE_ASSERT((uptr)mem % 128 == 0, "Memory aligment violation");
		MemoryArena header = {};
		header.free = size;
		header.offset = 0;
		header.begin = (void*)((byte*)mem + headerSize);
		header.stackMark = nullptr;
		header.size = size;
		CopyScalar(MemoryArena, mem, &header);
		return (MemoryArena*)mem;
	}

	void AppRun(Application* app)
	{
		AB_CORE_INFO("Aberration engine");

		app->state.runningTime = AB::GetCurrentRawTime();

		strcpy_s(app->windowTitle, WINDOW_TITLE_SIZE, "Aberration");
		app->state.windowWidth = 1280;
		app->state.windowHeight = 720;

		Win32Initialize(app);

		app->wglSwapIntervalEXT(1);

		LoadFunctionsResult glResult = OpenGLLoadFunctions(app->systemMemory);
		AB_CORE_ASSERT(glResult.success, "Failed to load OpenGL functions");
		InitOpenGL(glResult.funcTable);
		app->state.gl = glResult.funcTable;
		app->state.gl = glResult.funcTable;

		// TODO: !!!
		//app->state.functions.PlatformSetCursorPosition = WindowSetMousePosition;

		app->state.functions.ConsolePrint = ConsolePrint;
		app->state.functions.ConsoleSetColor = ConsoleSetColor;
		app->state.functions.DebugReadFilePermanent = DebugReadFilePermanent;
		app->state.functions.DebugGetFileSize = DebugGetFileSize;
		app->state.functions.DebugReadFile = DebugReadFileToBuffer;
		app->state.functions.DebugReadTextFile = DebugReadTextFileToBuffer;
		app->state.functions.GetLocalTime = GetLocalTime;
		
		char execPath[256];
		SetupDirs(execPath, 256,
				  app->libDir, MAX_GAME_LIB_PATH,
				  app->libFullPath, MAX_GAME_LIB_PATH);
		
		b32 codeLoaded = UpdateGameCode(app);
		AB_CORE_ASSERT(codeLoaded, "Failed to load code");

		app->gameMemory = AllocateArena(MEGABYTES(2048));
		
		i64 updateTimer = UPDATE_INTERVAL;
		i64 tickTimer = SECOND_INTERVAL;
		u32 updatesSinceLastTick = 0;
#if 0
		WindowPollEvents(&app->window);
		auto* inputState = &app->state.input;
		inputState->activeApp = app->window.activeWindow;
		inputState->mouseInWindow = app->window.mouseInClientArea;
#endif

		app->GameUpdateAndRender(app->gameMemory, &app->state,
										   GUR_REASON_INIT);
		AB::GetLocalTime(&app->state.localTime);
		while (app->running)
		{
			WindowPollEvents(app);

			if (tickTimer <= 0)
			{
				tickTimer = SECOND_INTERVAL;
				app->state.ups = updatesSinceLastTick;
				updatesSinceLastTick= 0;
			}

			if (updateTimer<= 0)
			{
				// TODO: move to render frequency?
				AB::GetLocalTime(&app->state.localTime);

				b32 codeReloaded = UpdateGameCode(app);
				if (codeReloaded)
				{
					app->GameUpdateAndRender(app->gameMemory,
											 &app->state,
											 GUR_REASON_RELOAD);				
				}
				updateTimer = UPDATE_INTERVAL;
				updatesSinceLastTick++;
				app->GameUpdateAndRender(app->gameMemory, &app->state,
										 GUR_REASON_UPDATE);
			}

			app->GameUpdateAndRender(app->gameMemory, &app->state,
									 GUR_REASON_RENDER);

			SwapBuffers(app->win32WindowDC);

			// NOTE: Cache hell!!!
			for (u32 keyIndex = 0; keyIndex < KEYBOARD_KEYS_COUNT; keyIndex ++)
			{
				app->state.input.keys[keyIndex].wasPressed =
					app->state.input.keys[keyIndex].pressedNow;
			}

			for (u32 mbIndex = 0; mbIndex < MOUSE_BUTTONS_COUNT; mbIndex++)
			{
				app->state.input.mouseButtons[mbIndex].wasPressed =
					app->state.input.mouseButtons[mbIndex].pressedNow;
			}

			app->state.input.scrollFrameOffset = 0;
			app->state.input.mouseFrameOffsetX = 0;
			app->state.input.mouseFrameOffsetY = 0;

			app->state.input.textBufferCount = 0;
			
			int64 current_time = GetCurrentRawTime();
			app->state.frameTime = current_time - app->state.runningTime;
			app->state.runningTime = current_time;
			tickTimer -= app->state.frameTime;
			updateTimer -= app->state.frameTime;
			app->state.fps = SECOND_INTERVAL / app->state.frameTime;
			app->state.absDeltaTime = app->state.frameTime / (1000.0f * 1000.0f);
			// NOTE : Temporary clamping delta time in order to avoid
			// glitches in time based code when frame time is too long
			if (app->state.absDeltaTime > 0.6f)
			{
				app->state.absDeltaTime = 0.6f;
			}
			app->state.gameDeltaTime = app->state.absDeltaTime * app->state.gameSpeed;
		}
	}
	
}

int main()
{
	AB::MemoryArena* sysArena = AB::AllocateArena(MEGABYTES(8));
	AB::Application* app = nullptr;
	app = (AB::Application*)AB::PushSize(sysArena, sizeof(AB::Application), 0);
	AB_CORE_ASSERT(app, "Failed to allocate Application.");
	app->systemMemory = sysArena;
	AB::AppRun(app);
	return 0;
}

#include "Win32Common.cpp"
#include "Win32CodeLoader.cpp"
#include "../OpenGLLoader.cpp"
#include "Log.cpp"
