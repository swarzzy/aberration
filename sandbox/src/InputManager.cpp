#include "InputManager.h"
#include <cstdlib>
#include <cstring>
#include "Application.h"

namespace AB {

	struct PlatformKeyInfo {
		byte currentState;
		byte prevState;
	};

	struct PlatformMouseButtonInfo {
		byte currentState;
		byte prevState;
	};
	
	InputPropeties* InputInitialize(Engine* context) {
		// TODO:: ALLOCATION
		InputPropeties* props;
		props = (InputPropeties*)malloc(sizeof(InputPropeties));
		memset(props, 0, sizeof(InputPropeties));
		// TODO: Unsafe set callback here because it can be changed everywhere.
		// Make callback arrays in the future? Or some event system?
		context->SetFocusCallback((void*)props, [](bool32 focus, void* data) {
			InputPropeties* properties = (InputPropeties*)data;
			properties->windowActive = focus;
			if (focus && properties->mouseMode == MouseMode::Captured) {
				Platform()->ShowCursor(false);
			} else {
				Platform()->ShowCursor(true);
			}
		});
		props->windowActive = context->WindowActive();
		return props;
	}

	void InputUpdate(InputPropeties* propeties) {

	}

	void SetMouseMode(InputPropeties* propeties, MouseMode mode) {
		propeties->mouseMode = mode;
		propeties->xMousePosition = 0.0f;
		propeties->yMousePosition = 0.0f;

		if (propeties->mouseMode == MouseMode::Captured) {
			Platform()->SetMouseMoveCallback(
				[](uint32 xPos, uint32 yPos, void* userData) {
				InputPropeties* props = (InputPropeties*)userData;
				if (props->windowActive) {
					uint32 winW;
					uint32 winH;
					Platform()->GetWindowSize(&winW, &winH);
					int32 xMid = winW / 2;
					int32 yMid = winH / 2;
					float32 xMouseFrameOffset = (float32)((int32)xPos - xMid);
					float32 yMouseFrameOffset = (float32)((int32)yPos - yMid);
					props->xMousePosition += xMouseFrameOffset;
					props->yMousePosition += yMouseFrameOffset;

					if (props->mouseMoveCallback) {
						props->mouseMoveCallback(MouseMode::Captured, xMouseFrameOffset, yMouseFrameOffset, props->mouseMoveCallbackUserData);
					}
					Platform()->SetMousePosition(xMid, yMid);
				}
			},
			(void*)propeties);
		} else if (propeties->mouseMode == MouseMode::Cursor) {
			Platform()->SetMouseMoveCallback([](uint32 xPos, uint32 yPos, void* userData) {
				InputPropeties* props = (InputPropeties*)userData;
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

	bool32 InputKeyHeld(InputPropeties* propeties, KeyboardKey key) {
		return Platform()->GetKeyState(key).currentState;
	}

	bool32 InputKeyPressed(InputPropeties* propeties, KeyboardKey key) {
		return Platform()->GetKeyState(key).currentState && !Platform()->GetKeyState(key).prevState;
	}

	bool32 MouseButtonPressed(InputPropeties* propeties, MouseButton button) {
		return Platform()->GetMouseButtonState(button).currentState;
	}

	void SetMouseMoveCallback(InputPropeties* propeties, void* userData, MouseMoveCallback* fn) {
		propeties->mouseMoveCallback = fn;
		propeties->mouseMoveCallbackUserData = userData;
	}
}
