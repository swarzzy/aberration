#pragma once
#include "../Types.h"

namespace AB {
	const uint32 GAMEPAD_BUTTONS_COUNT = 14;

	enum class GamepadButton : uint8 {
		A = 0, B, X, Y, LeftStick, RightStick,
		LeftButton, RightButton, Start, Back,
		DPadUp, DPadDown, DPadLeft, DPadRight
	};

	const uint32 MOUSE_BUTTONS_COUNT = 5;

	enum class MouseButton : uint8 {
		Left = 0, Right, Middle, XButton1, XButton2 
	};

	const uint32 KEYBOARD_KEYS_COUNT = 256;

#if defined (AB_PLATFORM_WINDOWS)

	enum class KeyboardKey : uint8 { // 65 - 90 : A-Z
		Space = 0x20,
		Apostrophe = 0xDE, //222
		Comma = 0xBC,	// 188
		Minus = 0xBD, //189
		Period = 0xBE, // 190
		Slash = 0xBF, // 191
		Key0 = 0x30,
		Key1 = 0x31,
		Key2 = 0x32,
		Key3 = 0x33,
		Key4 = 0x34,
		Key5 = 0x35,
		Key6 = 0x36,
		Key7 = 0x37,
		Key8 = 0x38,
		Key9 = 0x39,
		Semicolon = 0xBA, // 186
		Equal = 0xBB, // 187
		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,
		LeftBracket = 0xDB, // 219
		BackSlash = 0xDC, // 220
		RightBracket = 0xDD, // 221
		Tilde = 0xC0, // 192
		
		Escape = 0x1B,
		Enter = 0x0D, // 13
		Tab = 0x09,
		Backspace = 0x08,
		Insert = 0x2D,
		Delete = 0x2E,
		Right = 0x27,
		Left = 0x25,
		Down = 0x28,
		Up = 0x26,
		PageUp = 0x21,
		PageDown = 0x22,
		Home = 0x24,
		End = 0x23,
		CapsLock = 0x14,
		ScrollLock = 0x91,
		NumLock = 0x90,
		PrintScreen = 0x2C,
		Pause = 0x13,
		Return = 0x0D,
		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7A,
		F12 = 0x7B,
		F13 = 0x7C,
		F14 = 0x7D,
		F15 = 0x7E,
		F16 = 0x7F,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,
		NumPad0 = 0x60,
		NumPad1 = 0x61,
		NumPad2 = 0x62,
		NumPad3 = 0x63,
		NumPad4 = 0x64,
		NumPad5 = 0x65,
		NumPad6 = 0x66,
		NumPad7 = 0x67,
		NumPad8 = 0x68,
		NumPad9 = 0x69,
		NumPadDecimal = 0x6E,
		NumPadDivide = 0x6F,
		NumPadMultiply = 0x6A,
		NumPadSubtract = 0x6D,
		NumPadAdd = 0x6B,
		NumPadEnter = 0x0D, // 13
		//NumPadEqual = 336,
		LeftShift = 0xA0,
		LeftCtrl = 0xA2,
		LeftAlt = 0xA4,
		LeftSuper = 0x5B,
		Menu = 0xA5,
		RightShift = 0xA1,
		RightCtrl = 0xA3,
		RightAlt = 0xA5,
		RightSuper = 0x5C,
		Clear = 0x0C,
	};

#endif
}
