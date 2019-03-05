#include "InputManager.h"
#include <cstdlib>
#include <cstring>
#include "Application.h"
#include "Window.h"

namespace AB::InputManager {

	struct PlatformKeyInfo {
		byte currentState;
		byte prevState;
	};

	struct PlatformMouseButtonInfo {
		byte currentState;
		byte prevState;
	};
	
	Propeties* Initialize() {
		Propeties* props;
		props = (Propeties*)malloc(sizeof(Propeties));
		memset(props, 0, sizeof(Propeties));
		// TODO: Unsafe set callback here because it can be changed everywhere.
		// Make callback arrays in the future? Or some event system?
		Window::SetFocusCallback((void*)props, [](bool32 focus, void* data) {
			Propeties* properties = (Propeties*)data;
			properties->windowActive = focus;
			if (focus && properties->mouseMode == MouseMode::Captured) {
				Window::ShowCursor(false);
			} else {
				Window::ShowCursor(true);
			}
		});
		props->windowActive = Window::WindowActive();
		return props;
	}

	void Update(Propeties* propeties) {

	}

	void SetMouseMode(Propeties* propeties, MouseMode mode) {
		propeties->mouseMode = mode;
		propeties->xMousePosition = 0.0f;
		propeties->yMousePosition = 0.0f;

		if (propeties->mouseMode == MouseMode::Captured) {
			Window::SetMouseMoveCallback(
				[](uint32 xPos, uint32 yPos, void* userData) {
				Propeties* props = (Propeties*)userData;
				if (props->windowActive) {
					uint32 winW;
					uint32 winH;
					Window::GetSize(&winW, &winH);
					int32 xMid = winW / 2;
					int32 yMid = winH / 2;
					float32 xMouseFrameOffset = (float32)((int32)xPos - xMid);
					float32 yMouseFrameOffset = (float32)((int32)yPos - yMid);
					props->xMousePosition += xMouseFrameOffset;
					props->yMousePosition += yMouseFrameOffset;

					if (props->mouseMoveCallback) {
						props->mouseMoveCallback(MouseMode::Captured, xMouseFrameOffset, yMouseFrameOffset, props->mouseMoveCallbackUserData);
					}
					Window::SetMousePosition(xMid, yMid);
				}
			},
			(void*)propeties);
		} else if (propeties->mouseMode == MouseMode::Cursor) {
			Window::SetMouseMoveCallback([](uint32 xPos, uint32 yPos, void* userData) {
				Propeties* props = (Propeties*)userData;
				props->xMousePosition = (float32)xPos;
				props->yMousePosition = (float32)yPos;
				if (props->mouseMoveCallback) {
					props->mouseMoveCallback(MouseMode::Cursor, (float32)xPos, (float32)yPos, props->mouseMoveCallbackUserData);
				}
			},
			(void*)propeties);
		} else {
			// Unsupported mode
		}
	}

	bool32 KeyHeld(Propeties* propeties, KeyboardKey key) {
		return Window::GetKeyState(key).currentState;
	}

	bool32 KeyPressed(Propeties* propeties, KeyboardKey key) {
		return Window::GetKeyState(key).currentState && !Window::GetKeyState(key).prevState;
	}

	bool32 MouseButtonPressed(Propeties* propeties, MouseButton button) {
		return Window::GetMouseButtonState(button).currentState;
	}

	void SetMouseMoveCallback(Propeties* propeties, void* userData, MouseMoveCallback* fn) {
		propeties->mouseMoveCallback = fn;
		propeties->mouseMoveCallbackUserData = userData;
	}
}
