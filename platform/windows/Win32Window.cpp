#include "../Window.h"

#include "Log.h"

#include "Win32WGL.h"
#include "Memory.h"
#include <hypermath.h>

#include <Windows.h>
#include <Xinput.h>

// FORWARD DECLARATIONS
namespace AB::GL {
	bool32 LoadFunctions();
	bool32 LoadExtensions();
	void InitAPI();
}

// TODO:
// -- WGL_ARB_framebuffer_sRGB

// Macros from windowsx.h
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam) static_cast<uint16>(lParam & 0xffff >> 2)

// XInput functions definitions

typedef DWORD WINAPI _Win32XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD WINAPI _Win32XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

// Dummy functions for use if XInput loading failed. Is will work as is no device connected
inline static DWORD WINAPI __Win32XInputGetStateDummy(DWORD dwUserIndex, XINPUT_STATE* pState) {
	return ERROR_DEVICE_NOT_CONNECTED;
}

inline static DWORD WINAPI __Win32XInputSetStateDummy(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
	return ERROR_DEVICE_NOT_CONNECTED;
}

static _Win32XInputGetState* Win32XInputGetState = __Win32XInputGetStateDummy;
static _Win32XInputSetState* Win32XInputSetState = __Win32XInputSetStateDummy;

// ^^^ XInput functions definitions

namespace AB {
	static const char* AB_XINPUT_DLL = "xinput1_3.dll";
	static const char* WINDOW_CLASS_NAME = "Aberration Engine Win32";

	static constexpr uint32 OPENGL_MAJOR_VERSION = 4;
	static constexpr uint32 OPENGL_MINOR_VERSION = 5;

	static constexpr uint32 GAMEPAD_STATE_ARRAY_SIZE = GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT;
	
	struct GamepadAnalogCtrl {
		int16 leftStickX;
		int16 leftStickY;
		int16 rightStickX;
		int16 rightStickY;
		byte leftTrigger;
		byte rightTrigger;
		uint16 leftStickDeadZone;
		uint16 rightStickDeadZone;
		byte triggerDeadZone;
	};

	struct Win32KeyState {
		bool currentState;
		bool prevState;
		uint32 repeatCount;
	};

	static constexpr uint32 WINDOW_TITLE_SIZE = 32;

	struct WindowProperties {
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

		void* inputManager;
		PlatformMouseCallbackFn* PlatformMouseCallback;
		PlatformMouseButtonCallbackFn* PlatformMouseButtonCallback;
		PlatformKeyCallbackFn* PlatformKeyCallback;
		PlatformFocusCallbackFn* PlatformFocusCallback;
		PlatformMouseLeaveCallbackFn* PlatformMouseLeaveCallback;
		PlatformMouseScrollCallbackFn* PlatformMouseScrollCallback;

#if 0
		bool32 gamepadCurrentState[GAMEPAD_STATE_ARRAY_SIZE];
		bool32 gamepadPrevState[GAMEPAD_STATE_ARRAY_SIZE];
		GamepadAnalogCtrl gamepadAnalogControls[XUSER_MAX_COUNT];
		PlatformGamepadButtonCallback* gamepadButtonCallback;
		PlatformGamepadStickCallback* gamepadStickCallback;
		PlatformGamepadTriggerCallback* gamepadTriggerCallback;
#endif
		uint8 keyTable[KEYBOARD_KEYS_COUNT];
	};

	static LRESULT CALLBACK _Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	//static void _Win32GamepadUpdate(WindowProperties* props);
	//static void _Win32LoadXInput();
	static void _Win32InitOpenGL(WindowProperties* winProps);
	static void _Win32InitKeyTable(uint8* keytable);
	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint64 Win32Key);
	static void _Win32Initialize(WindowProperties* window);


	WindowProperties* WindowAllocateAndInit(MemoryArena* arena,
											const char* title,
											uint32 width, uint32 height)
	{
		WindowProperties* window = nullptr;
		window = (WindowProperties*)PushSize(arena,
											 sizeof(WindowProperties),
											 alignof(WindowProperties));
		AB_CORE_ASSERT(window, "Allocation failed");
		strcpy_s(window->title, WINDOW_TITLE_SIZE, title);
		window->width = width;
		window->height = height;
		_Win32Initialize(window);
		return window;
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

	void WindowRegisterInputManager(WindowProperties* window,
									void* inputManager,
									PlatformInputCallbacks* callbacks) {
		if (inputManager && callbacks)
		{
			window->inputManager = inputManager;
			window->PlatformMouseCallback = callbacks->PlatformMouseCallback;
			window->PlatformMouseButtonCallback = callbacks->PlatformMouseButtonCallback;
			window->PlatformKeyCallback = callbacks->PlatformKeyCallback;
			window->PlatformFocusCallback = callbacks->PlatformFocusCallback;
			window->PlatformMouseLeaveCallback = callbacks->PlatformMouseLeaveCallback;
			window->PlatformMouseScrollCallback = callbacks->PlatformMouseScrollCallback;
		}
		else
		{
			window->inputManager = nullptr;
			window->PlatformMouseCallback = nullptr;
			window->PlatformMouseButtonCallback = nullptr;
			window->PlatformKeyCallback = nullptr;
			window->PlatformFocusCallback = nullptr;
			window->PlatformMouseLeaveCallback = nullptr;
			window->PlatformMouseScrollCallback = nullptr;
		}
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

	bool WindowMouseInClientArea(WindowProperties* window) {
		return window->mouseInClientArea;
	}

#if 0
	bool WindowGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button) {
		auto window = PermStorage()->window;

		if (gamepadNumber >= XUSER_MAX_COUNT)
			return false;
		return window->gamepadCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)]
			&& !window->gamepadPrevState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
	}

	bool WindowGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button) {
		auto window = PermStorage()->window;

		if (gamepadNumber >= XUSER_MAX_COUNT)
			return false;
		return !window->gamepadCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)]
			&& window->gamepadPrevState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
	}

	bool WindowGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button) {
		auto window = PermStorage()->window;

		if (gamepadNumber < XUSER_MAX_COUNT)
			return window->gamepadCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
		else
			return 0;
	}

	void WindowGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY) {
		auto window = PermStorage()->window;

		if (gamepadNumber < XUSER_MAX_COUNT) {
			leftX = window->gamepadAnalogControls[gamepadNumber].leftStickX;
			leftY = window->gamepadAnalogControls[gamepadNumber].leftStickY;
			rightX = window->gamepadAnalogControls[gamepadNumber].rightStickX;
			rightY = window->gamepadAnalogControls[gamepadNumber].rightStickY;
		}
	}

	void WindowGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt) {
		auto window = PermStorage()->window;

		if (gamepadNumber < XUSER_MAX_COUNT) {
			lt = window->gamepadAnalogControls[gamepadNumber].leftTrigger;
			rt = window->gamepadAnalogControls[gamepadNumber].rightTrigger;
		}
	}

	void WindowSetGamepadButtonCallback(PlatformGamepadButtonCallback* func) {
		auto window = PermStorage()->window;

		window->gamepadButtonCallback = func;
	}

	void WindowSetGamepadStickCallback(PlatformGamepadStickCallback* func) {
		auto window = PermStorage()->window;

		window->gamepadStickCallback = func;
	}

	void WindowSetGamepadTriggerCallback(PlatformGamepadTriggerCallback* func) {
		auto window = PermStorage()->window;

		window->gamepadTriggerCallback = func;
	}
#endif

	void _Win32Initialize(WindowProperties* window) {
		auto instance = GetModuleHandle(0);

		WNDCLASS windowClass = {};
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = _Win32WindowCallback;
		windowClass.hInstance = instance;
		windowClass.lpszClassName = WINDOW_CLASS_NAME;

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
			hpm::Abs(actualSize.left) + hpm::Abs(actualSize.right),
			hpm::Abs(actualSize.top) + hpm::Abs(actualSize.bottom),
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

		//_Win32LoadXInput();

		//GL::InitAPI();
#if 0
		// TODO: Move to function
		for (uint32 i = 0; i < XUSER_MAX_COUNT; i++) {
			window->gamepadAnalogControls[i].leftStickDeadZone =
				XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			window->gamepadAnalogControls[i].rightStickDeadZone =
				XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			window->gamepadAnalogControls[i].triggerDeadZone =
				XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		}
#endif
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
				//ab_delete_scalar window;
				//ab_delete_scalar window;
				PostQuitMessage(0);
			} break;

			case WM_CLOSE: {
				if (window->closeCallback)
					window->closeCallback();
				window->running = false;
				ShowWindow(window->Win32WindowHandle, SW_HIDE);
			} break;

				// MOUSE INPUT

			case WM_MOUSEMOVE: {
				if (!window->mouseInClientArea) {
					window->mouseInClientArea = true;
					TrackMouseEvent(&window->Win32MouseTrackEvent);
					if (window->inputManager && window->PlatformMouseLeaveCallback) {
						window->PlatformMouseLeaveCallback(window->inputManager, true);
					}
				}
				int32 mousePositionX = GET_X_LPARAM(lParam);
				int32 mousePositionY = GET_Y_LPARAM(lParam);
				if (window->inputManager && window->PlatformMouseCallback) {
					window->PlatformMouseCallback(window->inputManager,
												  mousePositionX,
												  window->height - mousePositionY,
												  window->width, window->height);
				}

			} break;

			case WM_LBUTTONDOWN: {
				if (window->inputManager && window->PlatformMouseButtonCallback) {
					window->PlatformMouseButtonCallback(window->inputManager,
														MouseButton::Left, true);
				}
			} break;

			case WM_LBUTTONUP: {
				if (window->inputManager && window->PlatformMouseButtonCallback) {
					window->PlatformMouseButtonCallback(window->inputManager,
														MouseButton::Left, false);
				}
			} break;

			case WM_RBUTTONDOWN: {
				if (window->inputManager && window->PlatformMouseButtonCallback) {
					window->PlatformMouseButtonCallback(window->inputManager,
														MouseButton::Right, true);
				}
			} break;

			case WM_RBUTTONUP: {
				if (window->inputManager && window->PlatformMouseButtonCallback) {
					window->PlatformMouseButtonCallback(window->inputManager,
														MouseButton::Right, false);
				}
			} break;

			case WM_MBUTTONDOWN: {
				if (window->inputManager && window->PlatformMouseButtonCallback) {
					window->PlatformMouseButtonCallback(window->inputManager,
														MouseButton::Middle, true);
				}
			} break;

			case WM_MBUTTONUP: {
				if (window->inputManager && window->PlatformMouseButtonCallback) {
					window->PlatformMouseButtonCallback(window->inputManager,
														MouseButton::Middle, false);
				}
			} break;

			case WM_XBUTTONDOWN: {
				auto state = HIWORD(wParam);
				if (state & XBUTTON1) {
					if (window->inputManager && window->PlatformMouseButtonCallback) { 
						window->PlatformMouseButtonCallback(window->inputManager,
															MouseButton::XButton1, true);
					}
				}
				else {
					if (window->inputManager && window->PlatformMouseButtonCallback) {
						window->PlatformMouseButtonCallback(window->inputManager,
															MouseButton::XButton2, true);
					}
				}
			} break;

			case WM_XBUTTONUP: {
				auto state = HIWORD(wParam);
				if (state & XBUTTON1) {
					if (window->inputManager && window->PlatformMouseButtonCallback) {
						window->PlatformMouseButtonCallback(window->inputManager,
															MouseButton::XButton1, false);
					}
				}
				else {
					if (window->inputManager && window->PlatformMouseButtonCallback) {
						window->PlatformMouseButtonCallback(window->inputManager,
															MouseButton::XButton2, false);
					}
				}
			} break;

			case WM_MOUSELEAVE: {
				window->mouseInClientArea = false;
				if (window->inputManager && window->PlatformMouseLeaveCallback) {
					window->PlatformMouseLeaveCallback(window->inputManager,
													   false);
				}
			} break;

			case WM_MOUSEWHEEL: {
				i32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
				i32 numSteps = delta / WHEEL_DELTA;
				if (window->inputManager && window->PlatformMouseScrollCallback) {
					window->PlatformMouseScrollCallback(window->inputManager,
														numSteps);
				}
			} break;

				// ^^^^ MOUSE INPUT

				// KEYBOARD INPUT

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				// TODO: Repeat counts for now doesnt working on windows
				uint32 key = _Win32KeyConvertToABKeycode(window, wParam);
				bool32 state = true;
				uint16 sys_repeat_count =  AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam);
				if (window->inputManager && window->PlatformKeyCallback) {
					window->PlatformKeyCallback(window->inputManager,
												static_cast<KeyboardKey>(key),
												state, sys_repeat_count);
				}
			} break;

			case WM_SYSKEYUP:
			case WM_KEYUP: {
				uint32 key = _Win32KeyConvertToABKeycode(window, wParam);
				bool32 state = false;
				uint16 sys_repeat_count = 0;
				if (window->inputManager && window->PlatformKeyCallback) {
					window->PlatformKeyCallback(window->inputManager,
												static_cast<KeyboardKey>(key),
												state, sys_repeat_count);
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
				if (window->inputManager && window->PlatformFocusCallback) {
					window->PlatformFocusCallback(window->inputManager,
												  wParam == TRUE ? true : false);
				}
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
	
#if 0
	static void _Win32GamepadUpdate(WindowProperties* props) {
		for (DWORD cIndex = 0; cIndex < XUSER_MAX_COUNT; cIndex++) {
			XINPUT_STATE cState;
			auto result = Win32XInputGetState(cIndex, &cState);
			if (result == ERROR_SUCCESS) {
				memcpy(props->gamepadPrevState, props->gamepadCurrentState, sizeof(bool32) * GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT);

				XINPUT_GAMEPAD* gamepad = &cState.Gamepad;

				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::A)] = gamepad->wButtons & XINPUT_GAMEPAD_A;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::B)] = gamepad->wButtons & XINPUT_GAMEPAD_B;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::X)] = gamepad->wButtons & XINPUT_GAMEPAD_X;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::Y)] = gamepad->wButtons & XINPUT_GAMEPAD_Y;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::LeftStick)] = gamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::RightStick)] = gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::LeftButton)] = gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::RightButton)] = gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::Start)] = gamepad->wButtons & XINPUT_GAMEPAD_START;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::Back)] = gamepad->wButtons & XINPUT_GAMEPAD_BACK;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadUp)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadDown)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadLeft)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				props->gamepadCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadRight)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

				props->gamepadAnalogControls[cIndex].leftStickX = gamepad->sThumbLX;
				props->gamepadAnalogControls[cIndex].leftStickY = gamepad->sThumbLY;
				props->gamepadAnalogControls[cIndex].rightStickX = gamepad->sThumbRX;
				props->gamepadAnalogControls[cIndex].rightStickY = gamepad->sThumbRY;

				props->gamepadAnalogControls[cIndex].leftTrigger = gamepad->bLeftTrigger;
				props->gamepadAnalogControls[cIndex].rightTrigger = gamepad->bRightTrigger;

				if (props->gamepadButtonCallback) {
					for (uint32 i = 0; i < GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT; i++) {
						if (props->gamepadCurrentState[i] != props->gamepadPrevState[i]) {
							auto btn = static_cast<uint8>(i % ((cIndex + 1) * GAMEPAD_BUTTONS_COUNT));
							props->gamepadButtonCallback(static_cast<uint8>(cIndex), static_cast<GamepadButton>(btn),
														 props->gamepadCurrentState[i], props->gamepadPrevState[i]);
						}
					}
				}

				if (props->gamepadStickCallback) {
					if (props->gamepadAnalogControls->leftStickDeadZone < std::abs(gamepad->sThumbLX) ||
						props->gamepadAnalogControls->leftStickDeadZone < std::abs(gamepad->sThumbLY) ||
						props->gamepadAnalogControls->rightStickDeadZone < std::abs(gamepad->sThumbRX) ||
						props->gamepadAnalogControls->rightStickDeadZone < std::abs(gamepad->sThumbRY))
					{
						props->gamepadStickCallback(static_cast<uint8>(cIndex), gamepad->sThumbLX, gamepad->sThumbLY,
													gamepad->sThumbRX, gamepad->sThumbRY);
					}
				}

				if (props->gamepadTriggerCallback) {
					if (props->gamepadAnalogControls->triggerDeadZone < gamepad->bLeftTrigger ||
						props->gamepadAnalogControls->triggerDeadZone < gamepad->bRightTrigger)
					{
						props->gamepadTriggerCallback(static_cast<uint8>(cIndex), gamepad->bLeftTrigger, gamepad->bRightTrigger);
					}
				}
			}
		}
	}
	
	static void _Win32LoadXInput() {
		// TODO: try another versions
		HMODULE xInputLibrary = LoadLibrary(AB_XINPUT_DLL);
		if (xInputLibrary) {
			Win32XInputGetState = reinterpret_cast<_Win32XInputGetState*>(GetProcAddress(xInputLibrary, "XInputGetState"));
			Win32XInputSetState = reinterpret_cast<_Win32XInputSetState*>(GetProcAddress(xInputLibrary, "XInputSetState"));
		}
		else {
			AB_CORE_ERROR("Failed to load xinput1_3.dll. Gamepad is not working");
		}
	}

#endif

	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint64 Win32Key) {
		if (Win32Key < KEYBOARD_KEYS_COUNT)
			return window->keyTable[Win32Key];
		return static_cast<uint8>(KeyboardKey::InvalidKey);
	}

	static void _Win32InitKeyTable(uint8* keytable) {
		memset(keytable, static_cast<uint8>(KeyboardKey::InvalidKey), KEYBOARD_KEYS_COUNT);

		keytable[0x08] = static_cast<uint8>(KeyboardKey::Backspace);
		keytable[0x09] = static_cast<uint8>(KeyboardKey::Tab);
		keytable[0x0c] = static_cast<uint8>(KeyboardKey::Clear);
		keytable[0x0d] = static_cast<uint8>(KeyboardKey::Enter);
		keytable[0x10] = static_cast<uint8>(KeyboardKey::Shift);
		keytable[0x11] = static_cast<uint8>(KeyboardKey::Ctrl);
		keytable[0x12] = static_cast<uint8>(KeyboardKey::Alt);
		keytable[0x13] = static_cast<uint8>(KeyboardKey::Pause);
		keytable[0x14] = static_cast<uint8>(KeyboardKey::CapsLock);
		keytable[0x1b] = static_cast<uint8>(KeyboardKey::Escape);
		keytable[0x20] = static_cast<uint8>(KeyboardKey::Space);
		keytable[0x21] = static_cast<uint8>(KeyboardKey::PageUp);
		keytable[0x22] = static_cast<uint8>(KeyboardKey::PageDown);
		keytable[0x23] = static_cast<uint8>(KeyboardKey::End);
		keytable[0x24] = static_cast<uint8>(KeyboardKey::Home);
		keytable[0x25] = static_cast<uint8>(KeyboardKey::Left);
		keytable[0x26] = static_cast<uint8>(KeyboardKey::Up);
		keytable[0x27] = static_cast<uint8>(KeyboardKey::Right);
		keytable[0x28] = static_cast<uint8>(KeyboardKey::Down);
		keytable[0x2c] = static_cast<uint8>(KeyboardKey::PrintScreen);
		keytable[0x2d] = static_cast<uint8>(KeyboardKey::Insert);
		keytable[0x2e] = static_cast<uint8>(KeyboardKey::Delete);
		keytable[0x30] = static_cast<uint8>(KeyboardKey::Key0);
		keytable[0x31] = static_cast<uint8>(KeyboardKey::Key1);
		keytable[0x32] = static_cast<uint8>(KeyboardKey::Key2);
		keytable[0x33] = static_cast<uint8>(KeyboardKey::Key3);
		keytable[0x34] = static_cast<uint8>(KeyboardKey::Key4);
		keytable[0x35] = static_cast<uint8>(KeyboardKey::Key5);
		keytable[0x36] = static_cast<uint8>(KeyboardKey::Key6);
		keytable[0x37] = static_cast<uint8>(KeyboardKey::Key7);
		keytable[0x38] = static_cast<uint8>(KeyboardKey::Key8);
		keytable[0x39] = static_cast<uint8>(KeyboardKey::Key9);
		keytable[0x41] = static_cast<uint8>(KeyboardKey::A);
		keytable[0x42] = static_cast<uint8>(KeyboardKey::B);
		keytable[0x43] = static_cast<uint8>(KeyboardKey::C);
		keytable[0x44] = static_cast<uint8>(KeyboardKey::D);
		keytable[0x45] = static_cast<uint8>(KeyboardKey::E);
		keytable[0x46] = static_cast<uint8>(KeyboardKey::F);
		keytable[0x47] = static_cast<uint8>(KeyboardKey::G);
		keytable[0x48] = static_cast<uint8>(KeyboardKey::H);
		keytable[0x49] = static_cast<uint8>(KeyboardKey::I);
		keytable[0x4a] = static_cast<uint8>(KeyboardKey::J);
		keytable[0x4b] = static_cast<uint8>(KeyboardKey::K);
		keytable[0x4c] = static_cast<uint8>(KeyboardKey::L);
		keytable[0x4d] = static_cast<uint8>(KeyboardKey::M);
		keytable[0x4e] = static_cast<uint8>(KeyboardKey::N);
		keytable[0x4f] = static_cast<uint8>(KeyboardKey::O);
		keytable[0x50] = static_cast<uint8>(KeyboardKey::P);
		keytable[0x51] = static_cast<uint8>(KeyboardKey::Q);
		keytable[0x52] = static_cast<uint8>(KeyboardKey::R);
		keytable[0x53] = static_cast<uint8>(KeyboardKey::S);
		keytable[0x54] = static_cast<uint8>(KeyboardKey::T);
		keytable[0x55] = static_cast<uint8>(KeyboardKey::U);
		keytable[0x56] = static_cast<uint8>(KeyboardKey::V);
		keytable[0x57] = static_cast<uint8>(KeyboardKey::W);
		keytable[0x58] = static_cast<uint8>(KeyboardKey::X);
		keytable[0x59] = static_cast<uint8>(KeyboardKey::Y);
		keytable[0x5a] = static_cast<uint8>(KeyboardKey::Z);
		keytable[0x5b] = static_cast<uint8>(KeyboardKey::LeftSuper);
		keytable[0x5c] = static_cast<uint8>(KeyboardKey::RightSuper);
		keytable[0x60] = static_cast<uint8>(KeyboardKey::NumPad0);
		keytable[0x61] = static_cast<uint8>(KeyboardKey::NumPad1);
		keytable[0x62] = static_cast<uint8>(KeyboardKey::NumPad2);
		keytable[0x63] = static_cast<uint8>(KeyboardKey::NumPad3);
		keytable[0x64] = static_cast<uint8>(KeyboardKey::NumPad4);
		keytable[0x65] = static_cast<uint8>(KeyboardKey::NumPad5);
		keytable[0x66] = static_cast<uint8>(KeyboardKey::NumPad6);
		keytable[0x67] = static_cast<uint8>(KeyboardKey::NumPad7);
		keytable[0x68] = static_cast<uint8>(KeyboardKey::NumPad8);
		keytable[0x69] = static_cast<uint8>(KeyboardKey::NumPad9);
		keytable[0x6a] = static_cast<uint8>(KeyboardKey::NumPadMultiply);
		keytable[0x6b] = static_cast<uint8>(KeyboardKey::NumPadAdd);
		keytable[0x6d] = static_cast<uint8>(KeyboardKey::NumPadSubtract);
		keytable[0x6e] = static_cast<uint8>(KeyboardKey::NumPadDecimal);
		keytable[0x6f] = static_cast<uint8>(KeyboardKey::NumPadDivide);
		keytable[0x70] = static_cast<uint8>(KeyboardKey::F1);
		keytable[0x71] = static_cast<uint8>(KeyboardKey::F2);
		keytable[0x72] = static_cast<uint8>(KeyboardKey::F3);
		keytable[0x73] = static_cast<uint8>(KeyboardKey::F4);
		keytable[0x74] = static_cast<uint8>(KeyboardKey::F5);
		keytable[0x75] = static_cast<uint8>(KeyboardKey::F6);
		keytable[0x76] = static_cast<uint8>(KeyboardKey::F7);
		keytable[0x77] = static_cast<uint8>(KeyboardKey::F8);
		keytable[0x78] = static_cast<uint8>(KeyboardKey::F9);
		keytable[0x79] = static_cast<uint8>(KeyboardKey::F10);
		keytable[0x7a] = static_cast<uint8>(KeyboardKey::F11);
		keytable[0x7b] = static_cast<uint8>(KeyboardKey::F12);
		keytable[0x7c] = static_cast<uint8>(KeyboardKey::F13);
		keytable[0x7d] = static_cast<uint8>(KeyboardKey::F14);
		keytable[0x7e] = static_cast<uint8>(KeyboardKey::F15);
		keytable[0x7f] = static_cast<uint8>(KeyboardKey::F16);
		keytable[0x80] = static_cast<uint8>(KeyboardKey::F17);
		keytable[0x81] = static_cast<uint8>(KeyboardKey::F18);
		keytable[0x82] = static_cast<uint8>(KeyboardKey::F19);
		keytable[0x83] = static_cast<uint8>(KeyboardKey::F20);
		keytable[0x84] = static_cast<uint8>(KeyboardKey::F21);
		keytable[0x85] = static_cast<uint8>(KeyboardKey::F22);
		keytable[0x86] = static_cast<uint8>(KeyboardKey::F23);
		keytable[0x87] = static_cast<uint8>(KeyboardKey::F24);
		keytable[0x90] = static_cast<uint8>(KeyboardKey::NumLock);
		keytable[0x91] = static_cast<uint8>(KeyboardKey::ScrollLock);
		keytable[0xa0] = static_cast<uint8>(KeyboardKey::LeftShift);
		keytable[0xa1] = static_cast<uint8>(KeyboardKey::RightShift);
		// Only Ctrl now works and processed by SYSKEY events
		//keytable[0xa2] = static_cast<uint8>(KeyboardKey::LeftCtrl);
		//keytable[0xa3] = static_cast<uint8>(KeyboardKey::RightCtrl);
		//keytable[0xa4] = static_cast<uint8>(KeyboardKey::LeftAlt); 0x11
		keytable[0xa5] = static_cast<uint8>(KeyboardKey::Menu);
		keytable[0xba] = static_cast<uint8>(KeyboardKey::Semicolon);
		keytable[0xbb] = static_cast<uint8>(KeyboardKey::Equal);
		keytable[0xbc] = static_cast<uint8>(KeyboardKey::Comma);
		keytable[0xbd] = static_cast<uint8>(KeyboardKey::Minus);
		keytable[0xbe] = static_cast<uint8>(KeyboardKey::Period);
		keytable[0xbf] = static_cast<uint8>(KeyboardKey::Slash);
		keytable[0xc0] = static_cast<uint8>(KeyboardKey::Tilde);
		keytable[0xdb] = static_cast<uint8>(KeyboardKey::LeftBracket);
		keytable[0xdc] = static_cast<uint8>(KeyboardKey::BackSlash);
		keytable[0xdd] = static_cast<uint8>(KeyboardKey::RightBracket);
		keytable[0xde] = static_cast<uint8>(KeyboardKey::Apostrophe);
	}
	
}
