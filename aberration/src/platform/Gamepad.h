#pragma once
#include "../Types.h"

namespace AB {
	const uint8 GAMEPAD_BUTTONS_COUNT = 14;

	enum class GamepadButton : uint8 {
		A = 0, B, X, Y, LeftStick, RightStick,
		LeftButton, RightButton, Start, Back,
		DPadUp, DPadDown, DPadLeft, DPadRight
	};
}
