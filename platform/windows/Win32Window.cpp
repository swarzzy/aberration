#include "../Window.h"

#include "Log.h"

#include "Win32WGL.h"
#include "Memory.h"
#include <hypermath.h>

#include <Windows.h>

// FORWARD DECLARATIONS

// Macros from windowsx.h
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam) static_cast<uint16>(lParam & 0xffff >> 2)

namespace AB {
	static const char* WINDOW_CLASS_NAME = "Aberration Engine Win32";

	static constexpr uint32 OPENGL_MAJOR_VERSION = 4;
	static constexpr uint32 OPENGL_MINOR_VERSION = 5;

	struct Win32KeyState {
		bool currentState;
		bool prevState;
		uint32 repeatCount;
	};

	static constexpr uint32 WINDOW_TITLE_SIZE = 32;

	struct WindowProperties
	{
		char title[32];
		uint32 width;
		uint32 height;
		bool32 running;
		HWND Win32WindowHandle;
		HDC Win32WindowDC;
		HGLRC OpenGLRC;
		PlatformCloseCallback* closeCallback;
		PlatformResizeCallback* resizeCallback;
		bool32 activeWindow;

		TRACKMOUSEEVENT Win32MouseTrackEvent;
		bool32 mouseInClientArea;

		InputState* inputStatePtr;
		//TODO: These
		//PlatformFocusCallbackFn* PlatformFocusCallback;
		//PlatformMouseLeaveCallbackFn* PlatformMouseLeaveCallback;

		uint8 keyTable[KEYBOARD_KEYS_COUNT];
	};

	static LRESULT CALLBACK _Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static void _Win32InitOpenGL(WindowProperties* winProps);
	static void _Win32InitKeyTable(uint8* keytable);
	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint64 Win32Key);
	static void _Win32Initialize(WindowProperties* window);

	
	inline static void ProcessMButtonEvent(InputState* input,
										   MouseButton button, b32 state) 
	{
		input->mouseButtons[button].wasPressed =
			input->mouseButtons[button].pressedNow;
		input->mouseButtons[button].pressedNow = state;
	}

	WindowProperties* WindowAllocate(MemoryArena* arena)
	{
		WindowProperties* window = nullptr;
		window = (WindowProperties*)PushSize(arena,
											 sizeof(WindowProperties),
											 alignof(WindowProperties));
		AB_CORE_ASSERT(window, "Allocation failed");
		return window;
	}

	void WindowInit(WindowProperties* window, const char* title,
					uint32 width, uint32 height)
	{
		strcpy_s(window->title, WINDOW_TITLE_SIZE, title);
		window->width = width;
		window->height = height;
		_Win32Initialize(window);		
	}

	void WindowDestroy(WindowProperties* window) {
		AB_CORE_FATAL("Cannot destroy window for now. System allocator cannot free");
		window->running = false;
		ShowWindow(window->Win32WindowHandle, SW_HIDE);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(window->OpenGLRC);
		DestroyWindow(window->Win32WindowHandle);
		UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(0));
		ReleaseDC(window->Win32WindowHandle, window->Win32WindowDC);
	}

	void WindowClose(WindowProperties* window) {
		window->running = false;
		ShowWindow(window->Win32WindowHandle, SW_HIDE);
	}

	bool WindowIsOpen(WindowProperties* window) {
		return window->running;
	}

	void WindowPollEvents(WindowProperties* window) {
	   	MSG message;
		BOOL result;
		while (window->running && (result = PeekMessage(&message, window->Win32WindowHandle, 0, 0, PM_REMOVE)) != 0) {
			if (result == -1) {
				AB_CORE_FATAL("Window recieve error message.");
			}
			else {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		//_Win32GamepadUpdate(window);
	}

	void WindowSwapBuffers(WindowProperties* window) {
		::SwapBuffers(window->Win32WindowDC);
	}

	void WindowEnableVSync(WindowProperties* window, bool32 enable) {
		if (enable)
			wglSwapIntervalEXT(1);
		else
			wglSwapIntervalEXT(0);
	}

	void WindowGetSize(WindowProperties* window, uint32* width, uint32* height) {
		*width = window->width;
		*height = window->height;
	}

	void WindowSetCloseCallback(WindowProperties* window,
								PlatformCloseCallback* func) {
		window->closeCallback = func;
	}

	void WindowSetResizeCallback(WindowProperties* window,
								 PlatformResizeCallback* func) {
		window->resizeCallback = func;
	}

	void WindowSetMousePosition(WindowProperties* window, uint32 x, uint32 y) {
		uint32 yFlipped = 0;
		if (y < window->height) {
			yFlipped = window->height - y;
			POINT pt = { (LONG)x, (LONG)yFlipped };
			if (ClientToScreen(window->Win32WindowHandle, &pt)) {
				SetCursorPos(pt.x, pt.y);
			}
		}
	}

	bool32 WindowActive(WindowProperties* window) {
		return window->activeWindow;
	}

	void WindowShowCursor(WindowProperties* window, bool32 show) {
		// TODO: Make this work (SetCursor())
		//::ShowCursor(show ? TRUE : FALSE);
	}

	void WindowSetInputStatePtr(WindowProperties* window, InputState* inputPtr)
	{
		window->inputStatePtr = inputPtr;
	}

	bool WindowMouseInClientArea(WindowProperties* window) {
		return window->mouseInClientArea;
	}

	void _Win32Initialize(WindowProperties* window) {
		auto instance = GetModuleHandle(0);

		WNDCLASS windowClass = {};
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = _Win32WindowCallback;
		windowClass.hInstance = instance;
		windowClass.lpszClassName = WINDOW_CLASS_NAME;
		windowClass.hCursor = LoadCursor(0 ,IDC_ARROW);

		auto RCresult = RegisterClass(&windowClass);
		AB_CORE_ASSERT(RCresult, "Failed to create window.");

		HWND fakeWindow = CreateWindowEx(
			NULL,
			windowClass.lpszClassName,
			"",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			1,
			1,
			NULL,
			NULL,
			instance,
			NULL
			);

		HDC fakeWindowDC = GetDC(fakeWindow);

		PIXELFORMATDESCRIPTOR fakeDesiredPixelFormat = {};
		fakeDesiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		fakeDesiredPixelFormat.nVersion = 1;
		fakeDesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		fakeDesiredPixelFormat.cColorBits = 32;
		fakeDesiredPixelFormat.cAlphaBits = 8;
		fakeDesiredPixelFormat.cDepthBits = 24;
		fakeDesiredPixelFormat.cStencilBits = 8;

		auto fakeActualPFIndex = ChoosePixelFormat(fakeWindowDC, &fakeDesiredPixelFormat);

		PIXELFORMATDESCRIPTOR fakeActualPixelFormat = {};
		DescribePixelFormat(fakeWindowDC, fakeActualPFIndex, sizeof(PIXELFORMATDESCRIPTOR), &fakeActualPixelFormat);
		SetPixelFormat(fakeWindowDC, fakeActualPFIndex, &fakeActualPixelFormat);

		HGLRC fakeGLRC = wglCreateContext(fakeWindowDC);
		auto resultMC = wglMakeCurrent(fakeWindowDC, fakeGLRC);
		AB_CORE_ASSERT(resultMC, "Failed to create OpenGL context.");
		// TODO: Should it release dc?
		//ReleaseDC(windowHandle, windowDC);

		auto wglLoadProcsResult = Win32::LoadWGLFunctions(fakeWindowDC);
		AB_CORE_ASSERT(wglLoadProcsResult, "Failed to load WGL extensions");

		// ACTUAL WINDOW

		RECT actualSize = {};
		actualSize.top = 0;
		actualSize.left = 0;
		actualSize.right = window->width;
		actualSize.bottom = window->height;

		AdjustWindowRectEx(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, NULL, NULL);

		HWND actualWindowHandle = CreateWindowEx(
			NULL,
			windowClass.lpszClassName,
			window->title,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hpm::AbsI32(actualSize.left) + hpm::AbsI32(actualSize.right),
			hpm::AbsI32(actualSize.top) + hpm::AbsI32(actualSize.bottom),
			NULL,
			NULL,
			instance,
			window
			);

		AB_CORE_ASSERT(actualWindowHandle, "Failed to create window.");

		HDC actualWindowDC = GetDC(actualWindowHandle);

		// ^^^^ ACTUAL WINDOW

		//int multisampling = window->multisampling ? WGL_SAMPLE_BUFFERS_ARB : 0;

		int attribList[] = {
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
		auto resultCPF = wglChoosePixelFormatARB(actualWindowDC, attribList, nullptr, 1, &actualPixelFormatID, &numFormats);
		AB_CORE_ASSERT(resultCPF, "Failed to initialize OpenGL extended context.");

		PIXELFORMATDESCRIPTOR actualPixelFormat = {};
		auto resultDPF = DescribePixelFormat(actualWindowDC, actualPixelFormatID, sizeof(PIXELFORMATDESCRIPTOR), &actualPixelFormat);
		AB_CORE_ASSERT(resultDPF, "Failed to initialize OpenGL extended context.");
		SetPixelFormat(actualWindowDC, actualPixelFormatID, &actualPixelFormat);

		int contextAttribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR_VERSION,
			WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR_VERSION,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		HGLRC actualGLRC = wglCreateContextAttribsARB(actualWindowDC, 0, contextAttribs);
		AB_CORE_ASSERT(actualGLRC, "Failed to initialize OpenGL extended context");

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(fakeGLRC);
		ReleaseDC(fakeWindow, fakeWindowDC);
		DestroyWindow(fakeWindow);

		resultMC = wglMakeCurrent(actualWindowDC, actualGLRC);
		AB_CORE_ASSERT(resultMC, "Failed to initialize OpenGL extended context");

		//bool32 glLoadResult = GL::LoadFunctions();
		//AB_CORE_ASSERT(glLoadResult, "Failed to load OpenGL");
		//bool32 glEXTLoadResult = GL::LoadExtensions();
		//AB_CORE_ASSERT(glEXTLoadResult, "Failed to load OpenGL extensions");

		window->Win32WindowHandle = actualWindowHandle;
		window->Win32WindowDC = actualWindowDC;
		window->OpenGLRC = actualGLRC;

		//window->mouseWin32TrackEvent
		window->Win32MouseTrackEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		window->Win32MouseTrackEvent.dwFlags = TME_LEAVE;
		window->Win32MouseTrackEvent.dwHoverTime = HOVER_DEFAULT;
		window->Win32MouseTrackEvent.hwndTrack = window->Win32WindowHandle;
		TrackMouseEvent(&window->Win32MouseTrackEvent);

		_Win32InitKeyTable(window->keyTable);

		SetFocus(window->Win32WindowHandle);

		//GL::InitAPI();
	}

	static LRESULT CALLBACK _Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		LRESULT result = 0;

		if (message == WM_CREATE) {
			CREATESTRUCT* props = reinterpret_cast<CREATESTRUCT*>(lParam);
			WindowProperties* window = reinterpret_cast<WindowProperties*>(props->lpCreateParams);
			if (window) {
				SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
				window->running = true;
			}

			return result;
		};

		auto ptr = GetWindowLongPtr(windowHandle, GWLP_USERDATA);

		if (ptr != 0) {
			WindowProperties* window = reinterpret_cast<WindowProperties*>(ptr);

			switch (message) {
			case WM_SIZE: {
				window->width = LOWORD(lParam);
				window->height = HIWORD(lParam);
				if (window->resizeCallback)
					window->resizeCallback(window->width, window->height);
				//RenderGroupResizeCallback(window->width, window->height);
			} break;

			case WM_DESTROY: {
				PostQuitMessage(0);
			} break;

			case WM_CLOSE: {
				if (window->closeCallback)
					window->closeCallback();
				window->running = false;
				ShowWindow(window->Win32WindowHandle, SW_HIDE);
			} break;

				// MOUSE INPUT

			case WM_MOUSEMOVE:
			{
				if (!window->mouseInClientArea)
				{
					window->mouseInClientArea = true;
					TrackMouseEvent(&window->Win32MouseTrackEvent);
					window->inputStatePtr->mouseInWindow = true;
				}
				int32 mousePositionX = GET_X_LPARAM(lParam);
				int32 mousePositionY = GET_Y_LPARAM(lParam);
				
				mousePositionY = window->height - mousePositionY;

				f32 normalizedMouseX = (f32)mousePositionX / (f32)window->width;
				f32 normalizedMouseY = (f32)mousePositionY / (f32)window->height;
				
				window->inputStatePtr->mouseFrameOffsetX =
					normalizedMouseX - window->inputStatePtr->mouseX;
				
				window->inputStatePtr->mouseFrameOffsetY =
					normalizedMouseY - window->inputStatePtr->mouseY;
				
				window->inputStatePtr->mouseX = normalizedMouseX;
				window->inputStatePtr->mouseY = normalizedMouseY;
				
			} break;

			case WM_LBUTTONDOWN:
			{
				ProcessMButtonEvent(window->inputStatePtr, MBUTTON_LEFT, true);
			} break;

			case WM_LBUTTONUP:
			{
				ProcessMButtonEvent(window->inputStatePtr, MBUTTON_LEFT, false);
			} break;

			case WM_RBUTTONDOWN:
			{
				ProcessMButtonEvent(window->inputStatePtr, MBUTTON_RIGHT, true);
			} break;

			case WM_RBUTTONUP:
			{
				ProcessMButtonEvent(window->inputStatePtr, MBUTTON_RIGHT, false);
			} break;

			case WM_MBUTTONDOWN:
			{
				ProcessMButtonEvent(window->inputStatePtr, MBUTTON_MIDDLE, true);
			} break;

			case WM_MBUTTONUP:
			{
				ProcessMButtonEvent(window->inputStatePtr, MBUTTON_MIDDLE, false);
			} break;

			case WM_XBUTTONDOWN:
			{
				auto state = HIWORD(wParam);
				if (state & XBUTTON1)
				{
					ProcessMButtonEvent(window->inputStatePtr,
									   MBUTTON_XBUTTON1, true);
				}
				else
				{
					ProcessMButtonEvent(window->inputStatePtr,
									   MBUTTON_XBUTTON2, true);					
				}
			} break;

			case WM_XBUTTONUP:
			{
				auto state = HIWORD(wParam);
				if (state & XBUTTON1)
				{
					ProcessMButtonEvent(window->inputStatePtr,
									   MBUTTON_XBUTTON1, false);	
				}
				else
				{
					ProcessMButtonEvent(window->inputStatePtr,
									   MBUTTON_XBUTTON2, false);	
				}
			} break;

			case WM_MOUSELEAVE: {
				window->mouseInClientArea = false;
				window->inputStatePtr->mouseInWindow = false;
			} break;

			case WM_MOUSEWHEEL: {
				i32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
				i32 numSteps = delta / WHEEL_DELTA;
				window->inputStatePtr->scrollOffset = numSteps;
				window->inputStatePtr->scrollFrameOffset = numSteps;
			} break;

				// ^^^^ MOUSE INPUT

				// KEYBOARD INPUT

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				// TODO: Repeat counts for now doesnt working on windows
				// TODO: Why they are not working and why is this TODO here?
				uint32 key = _Win32KeyConvertToABKeycode(window, wParam);
				bool32 state = true;
				uint16 sys_repeat_count =  AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam);
				window->inputStatePtr->keys[key].wasPressed =
					window->inputStatePtr->keys[key].pressedNow;
				window->inputStatePtr->keys[key].pressedNow = state;
			} break;

			case WM_SYSKEYUP:
			case WM_KEYUP: {
				uint32 key = _Win32KeyConvertToABKeycode(window, wParam);
				bool32 state = false;
				uint16 sys_repeat_count = 0;
				window->inputStatePtr->keys[key].wasPressed =
					window->inputStatePtr->keys[key].pressedNow;
				window->inputStatePtr->keys[key].pressedNow = state;
			} break;

			case WM_CHAR:
			{
				u32 textBufferCount = window->inputStatePtr->textBufferCount;
				char* textBuffer =
					window->inputStatePtr->textBuffer + textBufferCount;
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
						window->inputStatePtr->textBufferCount += (u32)ret;
					}
				}
			} break;


				// ^^^^ KEYBOARD INPUT

			case WM_ACTIVATEAPP: {
				if (wParam == TRUE) {
					window->activeWindow = true;
				}
				else {
					window->activeWindow = false;
				}
				AB_CORE_ASSERT(window->inputStatePtr,
							   "Set inputStatePtr before init window");
				window->inputStatePtr->activeApp = window->activeWindow;
			} break;
			default: {
				result = DefWindowProc(windowHandle, message, wParam, lParam);
			} break;
			}
		}
		else {
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}

		return result;
	}
	
	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint64 Win32Key) {
		if (Win32Key < KEYBOARD_KEYS_COUNT)
			return window->keyTable[Win32Key];
		return KEY_INVALIDKEY;
	}

	static void _Win32InitKeyTable(uint8* keytable) {
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
	
}
