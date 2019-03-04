#pragma once
#include <Aberration.h>

namespace AB {
	// TODO: Input manager currently doesn`t work in live code mode because of memory allocation
	enum class MouseMode : uint32 {
		Cursor = 0,
		Captured,
	};
	
	typedef void(MouseMoveCallback)(MouseMode mode, float32 x, float32 y, void* userData);

	struct InputPropeties {
		MouseMode mouseMode;
		float32 xMousePosition;
		float32 yMousePosition;
		MouseMoveCallback* mouseMoveCallback;
		void* mouseMoveCallbackUserData;
		bool32 windowActive;
	};

	enum KeyState : byte {
		Released = 0,
		Pressed,
		Held
	};

	
	// TODO: Consider about moving InputManager to a platform
	// Because it's queries too many things that actually platform specific like ShowCursor.
	// Touchscreen devices for example doesn't have a cursor

	InputPropeties* InputInitialize(Engine* context);
	void InputUpdate(InputPropeties* propeties);
	void SetMouseMode(InputPropeties* propeties, MouseMode mode);
	bool32 InputKeyHeld(InputPropeties* propeties, KeyboardKey key);
	bool32 InputKeyPressed(InputPropeties* propeties, KeyboardKey key);
	bool32 MouseButtonPressed(InputPropeties* propeties, MouseButton button);
	void SetMouseMoveCallback(InputPropeties* propeties, void* userData, MouseMoveCallback* fn);
}