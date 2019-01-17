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
inline static DWORD WINAPI _Win32XInputGetStateDummy(DWORD dwUserIndex, XINPUT_STATE* pState) {
	return ERROR_DEVICE_NOT_CONNECTED;
}

inline static DWORD WINAPI _Win32XInputSetStateDummy(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
	return ERROR_DEVICE_NOT_CONNECTED;
}

static _Win32XInputGetState* Win32XInputGetState = _Win32XInputGetStateDummy;
static _Win32XInputSetState* Win32XInputSetState = _Win32XInputSetStateDummy;

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

		Win32KeyState keys[KEYBOARD_KEYS_COUNT]; // 1-current state, 2-prev state, 3+ - repeat count
		std::function<void(KeyboardKey, bool, bool, uint32)> keyCallback;
	};

	LRESULT CALLBACK WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	static void GamepadUpdate(WindowProperties* props);
	static void LoadXInput();
	static void InitOpenGL(HWND windowHandle);

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
		GamepadUpdate(s_WindowProperties);

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
		windowClass.lpfnWndProc = WindowCallback;
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
		InitOpenGL(handle);


		//SetWindowLongPtr(window->windowHandle, GWLP_USERDATA, 0);

		//window->mouseWin32TrackEvent
		s_WindowProperties->Win32MouseTrackEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		s_WindowProperties->Win32MouseTrackEvent.dwFlags = TME_LEAVE;
		s_WindowProperties->Win32MouseTrackEvent.dwHoverTime = HOVER_DEFAULT;
		s_WindowProperties->Win32MouseTrackEvent.hwndTrack = s_WindowProperties->Win32WindowHandle;
		TrackMouseEvent(&s_WindowProperties->Win32MouseTrackEvent);

		SetFocus(s_WindowProperties->Win32WindowHandle);

		LoadXInput();

		// TODO: Move to function
		for (uint32 i = 0; i < XUSER_MAX_COUNT; i++) {
			s_WindowProperties->gamepadAnalogControls[i].leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			s_WindowProperties->gamepadAnalogControls[i].rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			s_WindowProperties->gamepadAnalogControls[i].triggerDeadZone = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		}
	}

	LRESULT CALLBACK WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
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

			case WM_KEYDOWN: {
				window->keys[wParam].prevState = window->keys[wParam].currentState;
				window->keys[wParam].currentState = true;
				window->keys[wParam].repeatCount += AB_KEY_REPEAT_COUNT_FROM_LPARAM(lParam);
				if (window->keyCallback)
					window->keyCallback(static_cast<KeyboardKey>(wParam), true, window->keys[wParam].prevState, window->keys[wParam].repeatCount);
			} break;

			case WM_KEYUP: {
				window->keys[wParam].prevState = window->keys[wParam].currentState;
				window->keys[wParam].currentState = false;
				window->keys[wParam].repeatCount = 0;
				if (window->keyCallback)
					window->keyCallback(static_cast<KeyboardKey>(wParam), false, window->keys[wParam].prevState, window->keys[wParam].repeatCount);
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

	void GamepadUpdate(WindowProperties* props) {
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

	void LoadXInput() {
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

	static void InitOpenGL(HWND windowHandle) {
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
}














//#if defined(AB_PLATFORM_WINDOWS)
//#include "windows/Win32Window.h"
//
//namespace AB {
//	static Win32::Window* WindowInfo = nullptr;
//
//	AB_API void WindowCreate(const String& title, uint32 width, uint32 height) {
//		if (WindowInfo != nullptr)
//			AB_CORE_ERROR("Windows already created.");
//		else {
//			WindowInfo = Win32::WindowCreate(title, width, height);
//			AB_CORE_ASSERT(WindowInfo, "Failed to create Win32 window.");
//		}
//	}
//
//	AB_API void WindowPollEvents() {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::WindowPollEvents(WindowInfo);
//	}
//
//	AB_API bool WindowIsOpen() {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::WindowIsOpen(WindowInfo);
//		}
//	}
//
//	AB_API void WindowGetSize(uint32& width, uint32& height) {
//		if (!WindowInfo)
//			AB_CORE_WARN("Window is not created.");
//		else
//			Win32::WindowGetSize(WindowInfo, width, height);
//	}
//
//	AB_API void WindowSetCloseCallback(const std::function<void()>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::WindowSetCloseCallback(WindowInfo, func);
//	}
//
//	AB_API void WindowSetResizeCallback(const std::function<void(int, int)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::WindowSetResizeCallback(WindowInfo, func);
//	}
//
//	AB_API void WindowDestroy() {
//		if (!WindowInfo)
//			AB_CORE_WARN("Window is not created.");
//		else {
//			Win32::WindowDestroyWindow(WindowInfo);
//			WindowInfo = nullptr;
//		}
//	}
//
//	AB_API bool InputGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button) {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::InputGamepadButtonPressed(WindowInfo, gamepadNumber, button);
//		}
//	}
//
//	AB_API bool InputGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button) {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::InputGamepadButtonReleased(WindowInfo, gamepadNumber, button);
//		}
//	}
//
//	AB_API bool InputGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button) {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::InputGamepadButtonHeld(WindowInfo, gamepadNumber, button);
//		}
//	}
//
//	AB_API void InputSetGamepadButtonCallback(const std::function<void(uint8, GamepadButton, bool, bool)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputSetGamepadButtonCallback(WindowInfo, func);
//	}
//
//	AB_API void InputGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputGetGamepadStickPosition(WindowInfo, gamepadNumber, leftX, leftY, rightX, rightY);
//	}
//
//	AB_API void InputGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputGetGamepadStickPosition(WindowInfo, gamepadNumber, lt, rt);
//	}
//
//	AB_API void InputSetGamepadStickCallback(const std::function<void(uint8, int16, int16, int16, int16)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputSetGamepadStickCallback(WindowInfo, func);
//	}
//
//	AB_API void InputSetGamepadTriggerCallback(const std::function<void(uint8, byte, byte)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputSetGamepadTriggerCallback(WindowInfo, func);
//	}
//
//	AB_API void InputGetMousePosition(int32& xPos, int32& yPos) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputGetMousePosition(WindowInfo, xPos, yPos);
//	}
//
//	AB_API bool InputMouseButtonPressed(MouseButton button) {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::InputMouseButtonPressed(WindowInfo, button);
//		}
//	}
//
//	AB_API bool InputMouseInClientArea() {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::InputMouseInClientArea(WindowInfo);
//		}
//	}
//
//	AB_API void InputMouseCapture(bool capture) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputMouseCapture(WindowInfo,  capture);
//	}
//
//	AB_API void InputSetMouseButtonCallback(const std::function<void(MouseButton, bool)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputSetMouseButtonCallback(WindowInfo, func);
//	}
//
//	AB_API void InputSetMouseMoveCallback(const std::function<void(uint32, uint32)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputSetMouseMoveCallback(WindowInfo, func);
//	}
//
//	AB_API bool InputKeyPressed(KeyboardKey key) {
//		if (!WindowInfo) {
//			AB_CORE_WARN("Window is not created.");
//			return false;
//		}
//		else {
//			return Win32::InputKeyPressed(WindowInfo, key);
//		}
//	}
//
//	AB_API void InputKeySetCallback(const std::function<void(KeyboardKey, bool, bool, uint32)>& func) {
//		if (!WindowInfo)
//			AB_CORE_ERROR("Window is not created.");
//		else
//			Win32::InputKeySetCallback(WindowInfo, func);
//	}
//}
//#endif
