#pragma once
#include "AB.h"
#include "Input.h"
#include <hypermath.h>

namespace AB {
	constexpr uint32 MAX_EVENT_SUBSC = 32768;
	constexpr uint32 EVENT_QUEUE_SIZE = 32768;
	constexpr int32  EVENT_INVALID_HANDLE = -1;

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

	//
	// TODO: Maybe there are doesn't need to be an event queue.
	// Subscriber's callbacks may just be called at the time manager gets event.
	// So then all events actually will be handled in platform PollEvents method.
	//
	// TODO: Data oriented optimizations. Make event structs traversing cache friendly
	//

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

	struct InternalEventQuery {
		int32 handle;
		byte _pad[4];
		EventQuery query;
	};

	struct Keys {
		byte current_state[KEYBOARD_KEYS_COUNT];
		byte prev_state[KEYBOARD_KEYS_COUNT];
		// Isn't 65536 are to small for repeat count?
		uint16 sys_repeat_count[KEYBOARD_KEYS_COUNT];
	};

	struct MouseButtons {
		byte current_state[MOUSE_BUTTONS_COUNT];
		byte prev_state[MOUSE_BUTTONS_COUNT];
	};

	struct InputMgr {
		int32 sub_handle_counter;
		MouseMode mouse_mode;
		float32 mouse_pos_x;
		float32 mouse_pos_y;
		bool32 window_active;
		bool32 mouse_in_client_area;
		uint32 event_queue_at;
		uint32 last_subscription;
		MouseButtons mouse_buttons[MOUSE_BUTTONS_COUNT];
		Keys keys[KEYBOARD_KEYS_COUNT];
		Event event_queue[EVENT_QUEUE_SIZE];
		InternalEventQuery subscriptions[MAX_EVENT_SUBSC];
	};

	enum KeyState : byte {
		Released = 0,
		Pressed,
		Held
	};

	AB_API InputMgr* InputInitialize();
	AB_API void InputBeginFrame(InputMgr* mgr);
	AB_API void InputEndFrame(InputMgr* mgr);

	AB_API int32 InputSubscribeEvent(InputMgr* mgr, const EventQuery* query);
	AB_API void InputUnsubscribeEvent(InputMgr* mgr, int32 handle);


	AB_API hpm::Vector2 InputGetMousePosition(InputMgr* mgr);

	AB_API void InputSetMouseMode(InputMgr* mgr, MouseMode mode);
	AB_API bool32 InputKeyIsPressed(InputMgr* mgr, KeyboardKey key);
	AB_API bool32 InputKeyIsDown(InputMgr* mgr, KeyboardKey key);
	AB_API bool32 InputKeyIsReleased(InputMgr* mgr, KeyboardKey key);
	AB_API bool32 InputMouseButtonIsPressed(InputMgr* mgr, MouseButton button);
	AB_API bool32 InputMouseButtonIsDown(InputMgr* mgr, MouseButton button);
	AB_API bool32 InputMouseButtonIsReleased(InputMgr* mgr, MouseButton button);

}
