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

	enum class KeyboardKey : uint8 {
		InvalidKey = 0x00,
		// Currently works only Ctrl for both left and right keys
		// Right Ctrl and Super key doesn`t work on Linux.
		Ctrl,
		Space,
		Apostrophe,
		Comma,
		Minus, 
		Period, 
		Slash,
		Key0 = 0x30,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,
		Semicolon,
		Equal,
		A = 0x41,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		LeftBracket,
		BackSlash,
		RightBracket,
		Tilde,
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		Delete,
		Right,
		Left,
		Down,
		Up,
		PageUp,
		PageDown,
		Home,
		End,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		Return = Enter,
		F1 = 114,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,
		NumPadDecimal,
		NumPadDivide,
		NumPadMultiply,
		NumPadSubtract,
		NumPadAdd,
		NumPadEnter = Enter,
		LeftShift = 153,
		LeftCtrl,
		Alt,
		LeftSuper,
		Menu,
		RightShift,
		RightCtrl,
		//RightAlt,
		RightSuper,
		Clear,
		Shift,
	};
	// TODO: ToString for keys
	/*AB_API String ToString(KeyboardKey key) {
		switch (key) {
		case KeyboardKey::InvalidKey:		return String("InvalidKey");
		case KeyboardKey::Space:			return String("Space");
		case KeyboardKey::Apostrophe:		return String("Apostrophe");
		case KeyboardKey::Comma:			return String("Comma");
		case KeyboardKey::Minus:			return String("Minus");
		case KeyboardKey::Period:			return String("Period");
		case KeyboardKey::Slash:			return String("Slash");
		case KeyboardKey::Key0:				return String("Key0");
		case KeyboardKey::Key1:				return String("Key1");
		case KeyboardKey::Key2:				return String("Key2");
		case KeyboardKey::Key3:				return String("Key3");
		case KeyboardKey::Key4:				return String("Key4");
		case KeyboardKey::Key5:				return String("Key5");
		case KeyboardKey::Key6:				return String("Key6");
		case KeyboardKey::Key7:				return String("Key7");
		case KeyboardKey::Key8:				return String("Key8");
		case KeyboardKey::Key9:				return String("Key9");
		case KeyboardKey::Semicolon:		return String("Semicolon");
		case KeyboardKey::Equal:			return String("Equal");
		case KeyboardKey::A:				return String("A");
		case KeyboardKey::B:				return String("B");
		case KeyboardKey::C:				return String("C");
		case KeyboardKey::D:				return String("D");
		case KeyboardKey::E:				return String("E");
		case KeyboardKey::F:				return String("F");
		case KeyboardKey::G:				return String("G");
		case KeyboardKey::H:				return String("H");
		case KeyboardKey::I:				return String("I");
		case KeyboardKey::J:				return String("J");
		case KeyboardKey::K:				return String("K");
		case KeyboardKey::L:				return String("L");
		case KeyboardKey::M:				return String("M");
		case KeyboardKey::N:				return String("N");
		case KeyboardKey::O:				return String("O");
		case KeyboardKey::P:				return String("P");
		case KeyboardKey::Q:				return String("Q");
		case KeyboardKey::R:				return String("R");
		case KeyboardKey::S:				return String("S");
		case KeyboardKey::T:				return String("T");
		case KeyboardKey::U:				return String("U");
		case KeyboardKey::V:				return String("V");
		case KeyboardKey::W:				return String("W");
		case KeyboardKey::X:				return String("X");
		case KeyboardKey::Y:				return String("Y");
		case KeyboardKey::Z:				return String("Z");
		case KeyboardKey::LeftBracket:		return String("LeftBracket");
		case KeyboardKey::BackSlash:		return String("BackSlash");
		case KeyboardKey::RightBracket:		return String("RightBracket");
		case KeyboardKey::Tilde:			return String("Tilde");
		case KeyboardKey::Escape:			return String("Escape");
		case KeyboardKey::Enter:			return String("Enter");
		case KeyboardKey::Tab:				return String("Tab");
		case KeyboardKey::Backspace:		return String("Backspace");
		case KeyboardKey::Insert:			return String("Insert");
		case KeyboardKey::Delete:			return String("Delete");
		case KeyboardKey::Right:			return String("Right");
		case KeyboardKey::Left:				return String("Left");
		case KeyboardKey::Down:				return String("Down");
		case KeyboardKey::Up:				return String("Up");
		case KeyboardKey::PageUp:			return String("PageUp");
		case KeyboardKey::PageDown:			return String("PageDown");
		case KeyboardKey::Home:				return String("Home");
		case KeyboardKey::End:				return String("End");
		case KeyboardKey::CapsLock:			return String("CapsLock");
		case KeyboardKey::ScrollLock:		return String("ScrollLock");
		case KeyboardKey::NumLock:			return String("NumLock");
		case KeyboardKey::PrintScreen:		return String("PrintScreen");
		case KeyboardKey::Pause:			return String("Pause");
		case KeyboardKey::F1:				return String("F1");
		case KeyboardKey::F2:				return String("F2");
		case KeyboardKey::F3:				return String("F3");
		case KeyboardKey::F4:				return String("F4");
		case KeyboardKey::F5:				return String("F5");
		case KeyboardKey::F6:				return String("F6");
		case KeyboardKey::F7:				return String("F7");
		case KeyboardKey::F8:				return String("F8");
		case KeyboardKey::F9:				return String("F9");
		case KeyboardKey::F10:				return String("F10");
		case KeyboardKey::F11:				return String("F11");
		case KeyboardKey::F12:				return String("F12");
		case KeyboardKey::F13:				return String("F13");
		case KeyboardKey::F14:				return String("F14");
		case KeyboardKey::F15:				return String("F15");
		case KeyboardKey::F16:				return String("F16");
		case KeyboardKey::F17:				return String("F17");
		case KeyboardKey::F18:				return String("F18");
		case KeyboardKey::F19:				return String("F19");
		case KeyboardKey::F20:				return String("F20");
		case KeyboardKey::F21:				return String("F21");
		case KeyboardKey::F22:				return String("F22");
		case KeyboardKey::F23:				return String("F23");
		case KeyboardKey::F24:				return String("F24");
		case KeyboardKey::NumPad0:			return String("NumPad0");
		case KeyboardKey::NumPad1:			return String("NumPad1");
		case KeyboardKey::NumPad2:			return String("NumPad2");
		case KeyboardKey::NumPad3:			return String("NumPad3");
		case KeyboardKey::NumPad4:			return String("NumPad4");
		case KeyboardKey::NumPad5:			return String("NumPad5");
		case KeyboardKey::NumPad6:			return String("NumPad6");
		case KeyboardKey::NumPad7:			return String("NumPad7");
		case KeyboardKey::NumPad8:			return String("NumPad8");
		case KeyboardKey::NumPad9:			return String("NumPad9");
		case KeyboardKey::NumPadDecimal:	return String("NumPadDecimal");
		case KeyboardKey::NumPadDivide:		return String("NumPadDivide");
		case KeyboardKey::NumPadMultiply:	return String("NumPadMultiply");
		case KeyboardKey::NumPadSubtract:	return String("NumPadSubtract");
		case KeyboardKey::NumPadAdd:		return String("NumPadAdd");
		case KeyboardKey::LeftShift:		return String("LeftShift");
		case KeyboardKey::LeftCtrl:			return String("LeftCtrl");
		case KeyboardKey::Alt:				return String("Alt");
		case KeyboardKey::LeftSuper:		return String("LeftSuper");
		case KeyboardKey::Menu:				return String("Menu");
		case KeyboardKey::RightShift:		return String("RightShift");
		case KeyboardKey::RightCtrl:		return String("RightCtrl");
		case KeyboardKey::RightSuper:		return String("RightSuper");
		case KeyboardKey::Clear:			return String("Clear");
		case KeyboardKey::Shift:			return String("Shift");
		case KeyboardKey::Ctrl:				return String("Ctrl");
		default:							return String("InvalidKey");
		}
	}*/
}
