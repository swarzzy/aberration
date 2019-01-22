#include "../Window.h"

#include "src/utils/Log.h"

#include <Windows.h>
#include <Xinput.h>
#include <gl/gl.h>

// Macros from windowsx.h
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

#define AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam) static_cast<uint32>(lParam & 0xffff >> 2)

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
	static const uint32 GAMEPAD_STATE_ARRAY_SIZE = GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT;

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

	struct WindowProperties {
		String title;
		uint32 width;
		uint32 height;
		bool running;
		HWND Win32WindowHandle;
		HDC Win32WindowDC;
		std::function<void()> closeCallback;
		std::function<void(uint32, uint32)> resizeCallback;

		TRACKMOUSEEVENT Win32MouseTrackEvent;
		int32 mousePositionX;
		int32 mousePositionY;
		std::function<void(MouseButton, bool)> mouseButtonCallback;
		std::function<void(uint32, uint32)> mouseMoveCallback;
		bool mouseButtonsCurrentState[MOUSE_BUTTONS_COUNT];
		bool mouseInClientArea;

		bool gamepadCurrentState[GAMEPAD_STATE_ARRAY_SIZE];
		bool gamepadPrevState[GAMEPAD_STATE_ARRAY_SIZE];
		GamepadAnalogCtrl gamepadAnalogControls[XUSER_MAX_COUNT];
		std::function<void(uint8, GamepadButton, bool, bool)> gamepadButtonCallback;
		std::function<void(uint8, int16, int16, int16, int16)> gamepadStickCallback;
		std::function<void(uint8, byte, byte)> gamepadTriggerCallback;

		std::function<void(KeyboardKey, bool, bool, uint32)> keyCallback;
		Win32KeyState keys[KEYBOARD_KEYS_COUNT]; // 1-current state, 2-prev state, 3+ - repeat count
		uint8 keyTable[KEYBOARD_KEYS_COUNT];
	};

	static LRESULT CALLBACK _Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static void _Win32GamepadUpdate(WindowProperties* props);
	static void _Win32LoadXInput();
	static void _Win32InitOpenGL(HWND windowHandle);
	static void _Win32InitKeyTable(uint8* keytable);
	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint64 Win32Key);

	void Window::Create(const String& title, uint32 width, uint32 height) {
		if (s_WindowProperties) {
			AB_CORE_WARN("Window already initialized");
			return;
		}

		s_WindowProperties = ab_create WindowProperties {};
		//memset(s_WindowProperties, 0, sizeof(WindowProperties));
		s_WindowProperties->title = title;
		s_WindowProperties->width = width;
		s_WindowProperties->height = height;

		PlatformCreate();
	}

	Window::~Window() {
		
	}

	void Window::Destroy() {
		ShowWindow(s_WindowProperties->Win32WindowHandle, SW_HIDE);
		DestroyWindow(s_WindowProperties->Win32WindowHandle);
		UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(0));
		// TODO: might be unsafe. It can call another methods before WM_DESTROY happens
		ab_delete_scalar s_WindowProperties;
		s_WindowProperties = nullptr;
	}

	bool Window::IsOpen() {
		return s_WindowProperties->running;
	}

	void Window::PollEvents() {
		MSG message;
		BOOL result;
		while (s_WindowProperties->running && (result = PeekMessage(&message, s_WindowProperties->Win32WindowHandle, 0, 0, PM_REMOVE)) != 0)
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
		_Win32GamepadUpdate(s_WindowProperties);

		// TODO: Temporary opengl stuff
		glViewport(0, 0, s_WindowProperties->width, s_WindowProperties->height);
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SwapBuffers(s_WindowProperties->Win32WindowDC);
	}

	void Window::GetSize(uint32& width, uint32& height) {
		width = s_WindowProperties->width;
		height = s_WindowProperties->height;
	}

	void Window::SetCloseCallback(const std::function<void()>& func) {
		s_WindowProperties->closeCallback = func;
	}

	void Window::SetResizeCallback(const std::function<void(uint32 width, uint32 height)>& func) {
		s_WindowProperties->resizeCallback = func;
	}

	bool Window::KeyPressed(KeyboardKey key) {
		return !(s_WindowProperties->keys[static_cast<uint8>(key)].prevState) && s_WindowProperties->keys[static_cast<uint8>(key)].currentState;
	}

	void Window::SetKeyCallback(const std::function<void(KeyboardKey key, bool currState, bool prevState, uint32 repeatCount)>& func) {
		s_WindowProperties->keyCallback = func;
	}

	void Window::GetMousePosition(uint32& xPos, uint32& yPos) {
		xPos = s_WindowProperties->mousePositionX;
		yPos = s_WindowProperties->mousePositionY;
	}

	bool Window::MouseButtonPressed(MouseButton button) {
		return s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(button)];
	}

	bool Window::MouseInClientArea() {
		return s_WindowProperties->mouseInClientArea;
	}

	void Window::SetMouseButtonCallback(const std::function<void(MouseButton btn, bool state)>& func) {
		s_WindowProperties->mouseButtonCallback = func;
	}

	void Window::SetMouseMoveCallback(const std::function<void(uint32 xPos, uint32 yPos)>& func) {
		s_WindowProperties->mouseMoveCallback = func;
	}

	bool Window::GamepadButtonPressed(uint8 gamepadNumber, GamepadButton button) {
		if (gamepadNumber >= XUSER_MAX_COUNT)
			return false;
		return s_WindowProperties->gamepadCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)]
			&& !s_WindowProperties->gamepadPrevState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
	}

	bool Window::GamepadButtonReleased(uint8 gamepadNumber, GamepadButton button) {
		if (gamepadNumber >= XUSER_MAX_COUNT)
			return false;
		return !s_WindowProperties->gamepadCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)]
			&& s_WindowProperties->gamepadPrevState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
	}

	bool Window::GamepadButtonHeld(uint8 gamepadNumber, GamepadButton button) {
		if (gamepadNumber < XUSER_MAX_COUNT)
			return s_WindowProperties->gamepadCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
		else
			return 0;
	}

	void Window::GetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX,int16& rightY) {
		if (gamepadNumber < XUSER_MAX_COUNT) {
			leftX = s_WindowProperties->gamepadAnalogControls[gamepadNumber].leftStickX;
			leftY = s_WindowProperties->gamepadAnalogControls[gamepadNumber].leftStickY;
			rightX = s_WindowProperties->gamepadAnalogControls[gamepadNumber].rightStickX;
			rightY = s_WindowProperties->gamepadAnalogControls[gamepadNumber].rightStickY;
		}
	}

	void Window::GetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt) {
		if (gamepadNumber < XUSER_MAX_COUNT) {
			lt = s_WindowProperties->gamepadAnalogControls[gamepadNumber].leftTrigger;
			rt = s_WindowProperties->gamepadAnalogControls[gamepadNumber].rightTrigger;
		}
	}

	void Window::SetGamepadButtonCallback(const std::function<void(uint8 gpNumber, GamepadButton btn, bool currState, bool prevState)>& func) {
		s_WindowProperties->gamepadButtonCallback = func;
	}

	void Window::SetGamepadStickCallback(const std::function<void(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs)>& func) {
		s_WindowProperties->gamepadStickCallback = func;
	}

	void Window::SetGamepadTriggerCallback(const std::function<void(uint8 gpNumber, byte lt, byte rt)>& func) {
		s_WindowProperties->gamepadTriggerCallback = func;
	}

	void Window::PlatformCreate() {
		auto instance = GetModuleHandle(0);

		WNDCLASS windowClass = {};
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = _Win32WindowCallback;
		windowClass.hInstance = instance;
		windowClass.lpszClassName = WINDOW_CLASS_NAME;

		auto result = RegisterClass(&windowClass);
		AB_CORE_ASSERT(result, "Failed to create window.");

		RECT actualSize = {};
		actualSize.top = 0;
		actualSize.left = 0;
		actualSize.right = s_WindowProperties->width;
		actualSize.bottom = s_WindowProperties->height;

		AdjustWindowRectEx(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, NULL, NULL);

		char* name = reinterpret_cast<char*>(alloca(s_WindowProperties->title.size() + 1));
		memcpy(name, s_WindowProperties->title.c_str(), s_WindowProperties->title.size() + 1);

		auto handle = CreateWindowEx(
			NULL,
			windowClass.lpszClassName,
			name,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			abs(actualSize.left) + abs(actualSize.right),
			abs(actualSize.top) + abs(actualSize.bottom),
			NULL,
			NULL,
			instance,
			s_WindowProperties
		);

		AB_CORE_ASSERT(handle, "Failed to create window.");

		s_WindowProperties->Win32WindowHandle = handle;
		s_WindowProperties->Win32WindowDC = GetDC(handle);

		// OPENGL stuff
		_Win32InitOpenGL(handle);


		//SetWindowLongPtr(window->windowHandle, GWLP_USERDATA, 0);

		//window->mouseWin32TrackEvent
		s_WindowProperties->Win32MouseTrackEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		s_WindowProperties->Win32MouseTrackEvent.dwFlags = TME_LEAVE;
		s_WindowProperties->Win32MouseTrackEvent.dwHoverTime = HOVER_DEFAULT;
		s_WindowProperties->Win32MouseTrackEvent.hwndTrack = s_WindowProperties->Win32WindowHandle;
		TrackMouseEvent(&s_WindowProperties->Win32MouseTrackEvent);
		
		_Win32InitKeyTable(s_WindowProperties->keyTable);

		SetFocus(s_WindowProperties->Win32WindowHandle);

		_Win32LoadXInput();

		// TODO: Move to function
		for (uint32 i = 0; i < XUSER_MAX_COUNT; i++) {
			s_WindowProperties->gamepadAnalogControls[i].leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			s_WindowProperties->gamepadAnalogControls[i].rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			s_WindowProperties->gamepadAnalogControls[i].triggerDeadZone = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		}
	}

	static LRESULT CALLBACK _Win32WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		LRESULT result = 0;

		if (message == WM_CREATE) {
			CREATESTRUCT* props = reinterpret_cast<CREATESTRUCT*>(lParam);
			WindowProperties* window = reinterpret_cast<WindowProperties*>(props->lpCreateParams);
			SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			window->running = true;

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
				}
				window->mousePositionX = GET_X_LPARAM(lParam);
				window->mousePositionY = GET_Y_LPARAM(lParam);
				if (window->mouseMoveCallback)
					window->mouseMoveCallback(window->mousePositionX, window->mousePositionY);
			} break;

			case WM_LBUTTONDOWN: {
				window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Left)] = true;
				if (window->mouseButtonCallback)
					window->mouseButtonCallback(MouseButton::Left, true);
			} break;

			case WM_LBUTTONUP: {
				window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Left)] = false;
				if (window->mouseButtonCallback)
					window->mouseButtonCallback(MouseButton::Left, false);
			} break;

			case WM_RBUTTONDOWN: {
				window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Right)] = true;
				if (window->mouseButtonCallback)
					window->mouseButtonCallback(MouseButton::Right, true);
			} break;

			case WM_RBUTTONUP: {
				window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Right)] = false;
				if (window->mouseButtonCallback)
					window->mouseButtonCallback(MouseButton::Right, false);
			} break;

			case WM_MBUTTONDOWN: {
				window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Middle)] = true;
				if (window->mouseButtonCallback)
					window->mouseButtonCallback(MouseButton::Middle, true);
			} break;

			case WM_MBUTTONUP: {
				window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Middle)] = false;
				if (window->mouseButtonCallback)
					window->mouseButtonCallback(MouseButton::Middle, false);
			} break;

			case WM_XBUTTONDOWN: {
				auto state = HIWORD(wParam);
				if (state & XBUTTON1) {
					window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton1)] = true;
					if (window->mouseButtonCallback)
						window->mouseButtonCallback(MouseButton::XButton1, true);
				}
				else {
					window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton2)] = true;
					if (window->mouseButtonCallback)
						window->mouseButtonCallback(MouseButton::XButton2, true);
				}
			} break;

			case WM_XBUTTONUP: {
				auto state = HIWORD(wParam);
				if (state & XBUTTON1) {
					window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton1)] = false;
					if (window->mouseButtonCallback)
						window->mouseButtonCallback(MouseButton::XButton1, false);
				}
				else {
					window->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton2)] = false;
					if (window->mouseButtonCallback)
						window->mouseButtonCallback(MouseButton::XButton2, false);
				}
			} break;

			case WM_MOUSELEAVE: {
				window->mouseInClientArea = false;
			} break;

			// ^^^^ MOUSE INPUT
			
			// KEYBOARD INPUT

			case WM_SYSKEYDOWN:

			case WM_SYSKEYUP:
				
			case WM_KEYDOWN: {
				uint32 key = _Win32KeyConvertToABKeycode(window, wParam);
				window->keys[key].prevState = window->keys[key].currentState;
				window->keys[key].currentState = true;
				window->keys[key].repeatCount += AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam);
				if (window->keyCallback)
					window->keyCallback(static_cast<KeyboardKey>(key), true, window->keys[key].prevState, window->keys[key].repeatCount);
			} break;

			case WM_KEYUP: {
				uint32 key = _Win32KeyConvertToABKeycode(window, wParam);
				window->keys[key].prevState = window->keys[key].currentState;
				window->keys[key].currentState = false;
				if (window->keyCallback)
					window->keyCallback(static_cast<KeyboardKey>(key), false, window->keys[key].prevState, window->keys[key].repeatCount);
				window->keys[key].repeatCount = 0;
			} break;

			// ^^^^ KEYBOARD INPUT

			case WM_ACTIVATEAPP: {

			} break;

			case WM_PAINT: {
				result = DefWindowProc(windowHandle, message, wParam, lParam);
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

	static void _Win32GamepadUpdate(WindowProperties* props) {
		for (DWORD cIndex = 0; cIndex < XUSER_MAX_COUNT; cIndex++) {
			XINPUT_STATE cState;
			auto result = Win32XInputGetState(cIndex, &cState);
			if (result == ERROR_SUCCESS) {
				memcpy(props->gamepadPrevState, props->gamepadCurrentState, sizeof(bool) * GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT);

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

	static void _Win32InitOpenGL(HWND windowHandle) {
		HDC windowDC = GetDC(windowHandle);

		PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
		desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		desiredPixelFormat.nVersion = 1;
		desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		desiredPixelFormat.cColorBits = 32;
		desiredPixelFormat.cAlphaBits = 8;
		desiredPixelFormat.cDepthBits = 24;
		desiredPixelFormat.cStencilBits = 8;

		auto actualPFIndex = ChoosePixelFormat(windowDC, &desiredPixelFormat);

		PIXELFORMATDESCRIPTOR actualPixelFormat = {};
		DescribePixelFormat(windowDC, actualPFIndex, sizeof(PIXELFORMATDESCRIPTOR), &actualPixelFormat);
		SetPixelFormat(windowDC, actualPFIndex, &actualPixelFormat);

		// TODO: delete context wglDeleteContext
		HGLRC glRC = wglCreateContext(windowDC);
		auto result = wglMakeCurrent(windowDC, glRC);
		AB_CORE_ASSERT(result, "Failed to create OpenGL context.");
		ReleaseDC(windowHandle, windowDC);
	}

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