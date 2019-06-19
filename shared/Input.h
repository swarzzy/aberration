#pragma once
#include "Shared.h"

namespace AB
{

	const u32 MOUSE_BUTTONS_COUNT = 5;

	enum MouseButton : u8
	{
		MBUTTON_LEFT = 0, MBUTTON_RIGHT,
		MBUTTON_MIDDLE, MBUTTON_XBUTTON1, MBUTTON_XBUTTON2 
	};

	const u32 KEYBOARD_KEYS_COUNT = 256;

	enum KeyCode : u8
	{
		KEY_INVALIDKEY = 0x00,
		// Currently works only Ctrl for both left and right keys
		// Right Ctrl and Super key doesn`t work on Linux.
		KEY_CTRL,
		KEY_SPACE,
		KEY_APOSTROPHE,
		KEY_COMMA,
		KEY_MINUS, 
		KEY_PERIOD, 
		KEY_SLASH,
		KEY_KEY0 = 0x30,
		KEY_KEY1,
		KEY_KEY2,
		KEY_KEY3,
		KEY_KEY4,
		KEY_KEY5,
		KEY_KEY6,
		KEY_KEY7,
		KEY_KEY8,
		KEY_KEY9,
		KEY_SEMICOLON,
		KEY_EQUAL,
		KEY_A = 0x41,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_LEFTBRACKET,
		KEY_BACKSLASH,
		KEY_RIGHTBRACKET,
		KEY_TILDE,
		KEY_ESCAPE,
		KEY_ENTER,
		KEY_TAB,
		KEY_BACKSPACE,
		KEY_INSERT,
		KEY_DELETE,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,
		KEY_PAGEUP,
		KEY_PAGEDOWN,
		KEY_HOME,
		KEY_END,
		KEY_CAPSLOCK,
		KEY_SCROLLLOCK,
		KEY_NUMLOCK,
		KEY_PRINTSCREEN,
		KEY_PAUSE,
		KEY_RETURN = KEY_ENTER,
		KEY_F1 = 114,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,
		KEY_F23,
		KEY_F24,
		KEY_NUMPAD0,
		KEY_NUMPAD1,
		KEY_NUMPAD2,
		KEY_NUMPAD3,
		KEY_NUMPAD4,
		KEY_NUMPAD5,
		KEY_NUMPAD6,
		KEY_NUMPAD7,
		KEY_NUMPAD8,
		KEY_NUMPAD9,
		KEY_NUMPADDECIMAL,
		KEY_NUMPADDIVIDE,
		KEY_NUMPADMULTIPLY,
		KEY_NUMPADSUBTRACT,
		KEY_NUMPADADD,
		KEY_NUMPADENTER = KEY_ENTER,
		KEY_LEFTSHIFT = 153,
		KEY_LEFTCTRL,
		KEY_ALT,
		KEY_LEFTSUPER,
		KEY_MENU,
		KEY_RIGHTSHIFT,
		KEY_RIGHTCTRL,
		//RightAlt,
		KEY_RIGHTSUPER,
		KEY_CLEAR,
		KEY_SHIFT,
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
