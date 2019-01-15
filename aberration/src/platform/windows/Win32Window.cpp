#include "Win32Window.h"

namespace AB::Win32 {

	LRESULT CALLBACK WindowCallback(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		LRESULT result = 0;

		if (message == WM_CREATE) {
			CREATESTRUCT* props = reinterpret_cast<CREATESTRUCT*>(lParam);
			Window* window = reinterpret_cast<Window*>(props->lpCreateParams);
			SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			window->running = true;
			return result;
		};

		auto ptr = GetWindowLongPtr(windowHandle, GWLP_USERDATA);
		
		if (ptr != 0) {
			Window* window = reinterpret_cast<Window*>(ptr);

			switch (message) {
			case WM_SIZE: {
				window->width = LOWORD(lParam);
				window->height = HIWORD(lParam);
				if (window->resizeCallback)
					window->resizeCallback(window->width, window->height);
			} break;

			case WM_DESTROY: {
				AB_DELS window;
				PostQuitMessage(0);
			} break;

			case WM_CLOSE: {
				if (window->closeCallback)
					window->closeCallback();
				window->running = false;
				ShowWindow(window->windowHandle, SW_HIDE);
				//AB_DELS window;
				//DestroyWindow(windowHandle);
			} break;

			case WM_ACTIVATEAPP: {

			} break;

			case WM_PAINT: {
				result = DefWindowProc(windowHandle, message, wParam, lParam);
			} break;

			default: {
				result = DefWindowProc(windowHandle, message, wParam, lParam);
			} break;
			}
		} else {
			result = DefWindowProc(windowHandle, message, wParam, lParam);
		}

		return result;
	}


	AB_API Window* WindowCreate(const String& title, uint32 width, uint32 height) {
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
		actualSize.right = width;
		actualSize.bottom = height;

		AdjustWindowRectEx(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, NULL, NULL);

		char* name = reinterpret_cast<char*>(alloca(title.size() + 1));
		memcpy(name, title.c_str(), title.size() + 1);

		Window* window = AB_NEW Window;
		memset(window, 0, sizeof(Window));
		window->width = width;
		window->height = height;
		window->title = title;

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
			window
		);

		window->windowHandle = handle;

		AB_CORE_ASSERT(window->windowHandle, "Failed to create window.");

		//SetWindowLongPtr(window->windowHandle, GWLP_USERDATA, 0);

		SetFocus(window->windowHandle);

		LoadXInput();

		// TODO: Move to function
		for (uint32 i = 0; i < XUSER_MAX_COUNT; i++) {
			window->gamepadsAnalogControls[i].leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			window->gamepadsAnalogControls[i].rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			window->gamepadsAnalogControls[i].triggerDeadZone = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		}

		return window;
	}

	AB_API void WindowPollEvents(Window* window) {
		MSG message;
		BOOL result;
		while (window->running && (result = PeekMessage(&message, window->windowHandle, 0, 0, PM_REMOVE)) != 0)
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
		GamepadUpdate(window);
	}

	AB_API bool WindowIsOpen(const Window* window) {
		return window->running;
	}

	AB_API void WindowGetSize(const Window* window, uint32& width, uint32& height) {
		width = window->width;
		height = window->height;
	}

	AB_API void WindowSetCloseCallback(Window* window, const std::function<void()>& func) {
		window->closeCallback = func;
	}

	AB_API void WindowSetResizeCallback(Window* window, const std::function<void(int, int)>& func) {
		window->resizeCallback = func;
	}

	AB_API void WindowDestroyWindow(Window* window) {
		//SendMessage(window->windowHandle, WM_CLOSE, NULL, NULL);
		ShowWindow(window->windowHandle, SW_HIDE);
		DestroyWindow(window->windowHandle);
		UnregisterClass(WINDOW_CLASS_NAME, GetModuleHandle(0));
	}

	void LoadXInput() {
		HMODULE xInputLibrary = LoadLibrary(AB_XINPUT_DLL);
		if (xInputLibrary) {
			Win32XInputGetState = reinterpret_cast<_Win32XInputGetState*>(GetProcAddress(xInputLibrary, "XInputGetState"));
			Win32XInputSetState = reinterpret_cast<_Win32XInputSetState*>(GetProcAddress(xInputLibrary, "XInputSetState"));
		}
		else {
			AB_CORE_ERROR("Failed to load xinput1_3.dll. Gamepad is not working");
		}
	}

	void GamepadUpdate(Window* window) {
		for (DWORD cIndex = 0; cIndex < XUSER_MAX_COUNT; cIndex++) {
			XINPUT_STATE cState;
			auto result = Win32XInputGetState(cIndex, &cState);
			if (result == ERROR_SUCCESS) {
				memcpy(window->gamepadsPrevState, window->gamepadsCurrentState, sizeof(bool) * GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT);

				XINPUT_GAMEPAD* gamepad = &cState.Gamepad;
	
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::A)] = gamepad->wButtons & XINPUT_GAMEPAD_A;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::B)] = gamepad->wButtons & XINPUT_GAMEPAD_B;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::X)] = gamepad->wButtons & XINPUT_GAMEPAD_X;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::Y)] = gamepad->wButtons & XINPUT_GAMEPAD_Y;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::LeftStick)] = gamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::RightStick)] = gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::LeftButton)] = gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::RightButton)] = gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::Start)] = gamepad->wButtons & XINPUT_GAMEPAD_START;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::Back)] = gamepad->wButtons & XINPUT_GAMEPAD_BACK;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadUp)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadDown)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadLeft)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				window->gamepadsCurrentState[cIndex * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(GamepadButton::DPadRight)] = gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

				window->gamepadsAnalogControls[cIndex].leftStickX = gamepad->sThumbLX;
				window->gamepadsAnalogControls[cIndex].leftStickY = gamepad->sThumbLY;
				window->gamepadsAnalogControls[cIndex].rightStickX = gamepad->sThumbRX;
				window->gamepadsAnalogControls[cIndex].rightStickY = gamepad->sThumbRY;

				window->gamepadsAnalogControls[cIndex].leftTrigger = gamepad->bLeftTrigger;
				window->gamepadsAnalogControls[cIndex].rightTrigger = gamepad->bRightTrigger;

				if (window->gamepadButtonCallback) {
					for (uint32 i = 0; i < GAMEPAD_BUTTONS_COUNT * XUSER_MAX_COUNT; i++) {
						if (window->gamepadsCurrentState[i] != window->gamepadsPrevState[i]) {
							auto btn = static_cast<uint8>(i % ((cIndex + 1) * GAMEPAD_BUTTONS_COUNT));
							window->gamepadButtonCallback(static_cast<uint8>(cIndex), static_cast<GamepadButton>(btn), 
															window->gamepadsCurrentState[i], window->gamepadsPrevState[i]);
						}
					}
				}

				if (window->gamepadStickCallback) {
					if (window->gamepadsAnalogControls->leftStickDeadZone < std::abs(gamepad->sThumbLX) ||
						window->gamepadsAnalogControls->leftStickDeadZone < std::abs(gamepad->sThumbLY) ||
						window->gamepadsAnalogControls->rightStickDeadZone < std::abs(gamepad->sThumbRX) ||
						window->gamepadsAnalogControls->rightStickDeadZone < std::abs(gamepad->sThumbRY)) 
					{
						window->gamepadStickCallback(static_cast<uint8>(cIndex), gamepad->sThumbLX, gamepad->sThumbLY, 
														gamepad->sThumbRX, gamepad->sThumbRY);
					}
				}

				if (window->gamepadTriggerCallback) {
					if (window->gamepadsAnalogControls->triggerDeadZone < gamepad->bLeftTrigger ||
						window->gamepadsAnalogControls->triggerDeadZone < gamepad->bRightTrigger) 
					{
						window->gamepadTriggerCallback(static_cast<uint8>(cIndex), gamepad->bLeftTrigger, gamepad->bRightTrigger);
					}
				}
			}
		}
	}

	AB_API bool InputGamepadButtonPressed(const Window* window, uint8 gamepadNumber, GamepadButton button) {
		if (gamepadNumber >= XUSER_MAX_COUNT)
			return false;
		return window->gamepadsCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)]
				&& !window->gamepadsPrevState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
	}

	AB_API bool InputGamepadButtonReleased(const Window* window, uint8 gamepadNumber, GamepadButton button) {
		if (gamepadNumber >= XUSER_MAX_COUNT)
			return false;
		return !window->gamepadsCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)]
				&& window->gamepadsPrevState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
	}

	AB_API bool InputGamepadButtonHeld(const Window* window, uint8 gamepadNumber, GamepadButton button) {
		if (gamepadNumber < XUSER_MAX_COUNT)
			return window->gamepadsCurrentState[gamepadNumber * GAMEPAD_BUTTONS_COUNT + static_cast<uint8>(button)];
		else
			return 0;
	}

	AB_API void InputSetGamepadButtonCallback(Window* window, const std::function<void(uint8, GamepadButton, bool, bool)>& func) {
		window->gamepadButtonCallback = func;
	}

	AB_API void InputGetGamepadStickPosition(Window* window, uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY) {
		if (gamepadNumber < XUSER_MAX_COUNT) {
			leftX = window->gamepadsAnalogControls[gamepadNumber].leftStickX;
			leftY = window->gamepadsAnalogControls[gamepadNumber].leftStickY;
			rightX = window->gamepadsAnalogControls[gamepadNumber].rightStickX;
			rightY = window->gamepadsAnalogControls[gamepadNumber].rightStickY;
		}
	}

	AB_API void InputGetGamepadStickPosition(Window* window, uint8 gamepadNumber, byte& lt, byte& rt) {
		if (gamepadNumber < XUSER_MAX_COUNT) {
			lt = window->gamepadsAnalogControls[gamepadNumber].leftTrigger;
			rt = window->gamepadsAnalogControls[gamepadNumber].rightTrigger;
		}
	}

	AB_API void InputSetGamepadStickCallback(Window* window, const std::function<void(uint8, int16, int16, int16, int16)>& func) {
		window->gamepadStickCallback = func;
	}

	AB_API void InputSetGamepadTriggerCallback(Window* window, const std::function<void(uint8, byte, byte)>& func) {
		window->gamepadTriggerCallback = func;
	}
}