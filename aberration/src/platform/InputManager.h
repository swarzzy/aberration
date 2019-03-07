#pragma once
#include "AB.h"
#include "Input.h"
#include <hypermath.h>

namespace AB {
	constexpr uint32 MAX_EVENT_SUBSC = 32768;
	constexpr uint32 EVENT_QUEUE_SIZE = 32768;

	enum class MouseMode : byte {
		Cursor = 0,
		Captured,
	};

	enum EventType : byte {
		EVENT_TYPE_MOUSE_MOVED			= 1,
		EVENT_TYPE_KEY_PRESSED			= 1 << 1,
		EVENT_TYPE_KEY_REPEAT			= 1 << 2,
		EVENT_TYPE_KEY_RELEASED			= 1 << 3,
		EVENT_TYPE_MOUSE_BTN_PRESSED	= 1 << 4,
		EVENT_TYPE_MOUSE_BTN_RELEASED	= 1 << 5
	};

	// NOTE: There are potentially could be problems with aligment of these structs 
	// on different machine architectures.

	struct Event {
		union {
			struct {
				float32 x;
				float32 y;
				MouseMode mode;
				byte _unused[3];
			} mouse_moved_event;

			struct {
				uint16 sys_repeat_count;
				KeyboardKey key;
				byte _unused[9];
			} key_event;

			struct {
				MouseButton button;
				byte _unused[11];
			} mouse_button_event;
			byte _raw[12];
		};
		EventType type;
		byte _unused[3];
	};

	typedef void(EventCallback)(Event event);

	struct EventQuery {
		union {
			struct {
				KeyboardKey key;
			} key_event;

			struct {
				MouseButton button;
			} mouse_button_event;
			byte _raw[1];
		} condition;
		byte type;
		byte _reserved[6];
		//bool8 pass_through;
		EventCallback* callback;
	};

	struct KeyInfo {
		byte current_state;
		byte prev_state;
		// Isn't 65536 are to small for repeat count?
		uint16 sys_repeat_count;
	};

	struct MouseButtonInfo {
		byte current_state;
		byte prev_state;
	};

	struct InputMgr {
		MouseMode mouse_mode;
		float32 mouse_pos_x;
		float32 mouse_pos_y;
		bool32 window_active;
		bool32 mouse_in_client_area;
		uint32 event_queue_at;
		uint32 subscriptions_at;
		MouseButtonInfo mouse_buttons[MOUSE_BUTTONS_COUNT];
		KeyInfo keys[KEYBOARD_KEYS_COUNT];
		Event event_queue[EVENT_QUEUE_SIZE];
		EventQuery subscriptions[MAX_EVENT_SUBSC];
	};

	enum KeyState : byte {
		Released = 0,
		Pressed,
		Held
	};

	AB_API InputMgr* InputInitialize();

	AB_API void InputSubscribeEvent(InputMgr* mgr, const EventQuery* query);

	AB_API hpm::Vector2 InputGetMousePosition(InputMgr* mgr);

	AB_API void InputUpdate(InputMgr* mgr);
	AB_API void InputSetMouseMode(InputMgr* mgr, MouseMode mode);
	AB_API bool32 InputKeyIsDown(InputMgr* mgr, KeyboardKey key);
	AB_API bool32 InputMouseButtonIsDown(InputMgr* mgr, MouseButton button);
}
