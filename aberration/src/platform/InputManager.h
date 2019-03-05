#pragma once
#include "AB.h"
#include "Input.h"

namespace AB::InputManager {
	// TODO: Input manager currently doesn`t work in live code mode because of memory allocation
	enum class MouseMode : uint32 {
		Cursor = 0,
		Captured,
	};
	
	typedef void(MouseMoveCallback)(MouseMode mode, float32 x, float32 y, void* userData);

	struct Propeties {
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


	AB_API Propeties* Initialize();
	AB_API void Update(Propeties* propeties);
	AB_API void SetMouseMode(Propeties* propeties, MouseMode mode);
	AB_API bool32 KeyHeld(Propeties* propeties, KeyboardKey key);
	AB_API bool32 KeyPressed(Propeties* propeties, KeyboardKey key);
	AB_API bool32 MouseButtonPressed(Propeties* propeties, MouseButton button);
	AB_API void SetMouseMoveCallback(Propeties* propeties, void* userData, MouseMoveCallback* fn);
}