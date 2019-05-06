#include "InputManager.h"
#include "Memory.h"

#include <cstring>


namespace AB {

	static void DispatchEvent(InputMgr* mgr, const Event* e);

	void PlatformFocusCallback(void* mgr, bool32 focus)
	{
		((InputMgr*)mgr)->window_active = focus;
	}

	void PlatformKeyCallback(void* _mgr,
							 KeyboardKey key,
							 bool32 state,
							 uint16 sys_repeat_count)
	{
		InputMgr* mgr = (InputMgr*)_mgr;
		Keys* keys = mgr->keys;
		byte key_idx = (byte)key;

		keys->prev_state[key_idx] = keys->current_state[key_idx];

		if (state) { // key down
			keys->current_state[key_idx] = 1;
			keys->sys_repeat_count[key_idx] += sys_repeat_count;
		}
		else {      // key up
			keys->current_state[key_idx] = 0;
			keys->sys_repeat_count[key_idx] = 0;
		}

		Event e = {};
		if (state && !keys->prev_state[key_idx]) {
			e.type = EVENT_TYPE_KEY_PRESSED;
		}
		else if (state && keys->prev_state[key_idx]) {
			e.type = EVENT_TYPE_KEY_REPEAT;
		}
		else if (!state) {
			e.type = EVENT_TYPE_KEY_RELEASED;
		}

		e.key_event.key = key;
		e.key_event.sys_repeat_count = sys_repeat_count;

		DispatchEvent(mgr, &e);
	}

	void PlatformMouseLeaveCallback(void*_mgr,
									bool32 in_client_area)
	{
		InputMgr* mgr = (InputMgr*)_mgr;
		mgr->mouse_in_client_area = in_client_area;
	}

	void PlatformMouseButtonCallback(void*_mgr,
									 MouseButton button,
									 bool32 state)
	{
		InputMgr* mgr = (InputMgr*)_mgr;
		MouseButtons* btns = mgr->mouse_buttons;
		byte btn_idx = (byte)button;

		btns->prev_state[btn_idx] = btns->current_state[btn_idx];
		btns->current_state[btn_idx] = state;

		Event e = {};
		if (state && !btns->prev_state[btn_idx]) {
			e.type = EVENT_TYPE_MOUSE_BTN_PRESSED;
		}
		else if (!state) {
			e.type = EVENT_TYPE_MOUSE_BTN_RELEASED;
		}

		e.mouse_button_event.button = button;

		DispatchEvent(mgr, &e);
	}

	void PlatformMouseCallback(void* _mgr, uint32 x_pos, uint32 y_pos,
							   u32 winW, u32 winH)
	{
		InputMgr* mgr = (InputMgr*)_mgr;
		if (mgr->mouse_mode == MouseMode::Captured) {
			if (mgr->window_active) {
				int32 x_mid = winW / 2;
				int32 y_mid = winH / 2;
				float32 offset_x = (float32)((int32)x_pos - x_mid);
				float32 offset_y = (float32)((int32)y_pos - y_mid);
				mgr->mouse_pos_x += offset_x;
				mgr->mouse_pos_y += offset_y;
				mgr->mouse_frame_offset_x += offset_x;
				mgr->mouse_frame_offset_y += offset_y;

				Event e = {};
				e.type = EVENT_TYPE_MOUSE_MOVED;
				e.mouse_moved_event.mode = MouseMode::Captured;
				e.mouse_moved_event.x = offset_x;
				e.mouse_moved_event.y = offset_y;

				DispatchEvent(mgr, &e);
				PlatformSetCursorPosition(x_mid, y_mid);
			}
		} else if (mgr->mouse_mode == MouseMode::Cursor) {
			mgr->mouse_frame_offset_x += (float32)x_pos - mgr->mouse_pos_x;
			mgr->mouse_frame_offset_y += (float32)y_pos - mgr->mouse_pos_y;
			mgr->mouse_pos_x = (float32)x_pos;
			mgr->mouse_pos_y = (float32)y_pos;

			Event e = {};
			e.type = EVENT_TYPE_MOUSE_MOVED;
			e.mouse_moved_event.mode = MouseMode::Cursor;
			e.mouse_moved_event.x = (float32)x_pos;
			e.mouse_moved_event.y = (float32)y_pos;

			DispatchEvent(mgr, &e);
		}
	}

	void PlatformMouseScrollCallback(void* inputManager, i32 offset)
	{
		InputManager* manager = (InputManager*)inputManager;
		manager->mouseScrollOffset = offset;
		manager->mouseFrameScrollOffset = offset;

		// TODO: Dispatch event???
	}

	static void DispatchEvent(InputMgr* mgr, const Event* e) {
		for (uint32 q_index = 0; q_index <= mgr->last_subscription; q_index++) {
			InternalEventQuery* q = &mgr->subscriptions[q_index];
			if (q->handle != EVENT_INVALID_HANDLE) {
				if (e->type & q->query.type) {
					if (e->type == EVENT_TYPE_MOUSE_MOVED) {
						q->query.callback(*e);
						//if (!q->pass_through) {
						//	break;
						//}
					} else
					if (e->type == EVENT_TYPE_KEY_PRESSED ||
						e->type == EVENT_TYPE_KEY_RELEASED ||
						e->type == EVENT_TYPE_KEY_REPEAT) 
					{
						if (q->query.condition.key_event.key == e->key_event.key) {
							q->query.callback(*e);
							//if (!q->pass_through) {
							//	break;
							//}
						}
					} else
					if (e->type == EVENT_TYPE_MOUSE_BTN_PRESSED ||
						e->type == EVENT_TYPE_MOUSE_BTN_RELEASED) 
					{
						if (q->query.condition.mouse_button_event.button == e->mouse_button_event.button) {
							q->query.callback(*e);
						}
					}
				}
			}
		}
	}
		
	InputMgr* InputInitialize(MemoryArena* memory, PlatformState* platform)
	{
		InputMgr* mgr = nullptr;
		mgr = (InputMgr*)PushSize(memory, sizeof(InputMgr),
								  alignof(InputMgr));
		// TODO: Assert
		for (uint32 i = 0; i < MAX_EVENT_SUBSC; i++) {
			mgr->subscriptions[i].handle = EVENT_INVALID_HANDLE;
		}
		mgr->window_active = platform->windowActive;

		return mgr;
	}

	void InputConnectToPlatform(InputMgr* mgr, PlatformState* platform)
	{
		PlatformInputCallbacks callbacks = {};
		callbacks.PlatformMouseCallback = PlatformMouseCallback;
		callbacks.PlatformMouseButtonCallback = PlatformMouseButtonCallback;
		callbacks.PlatformKeyCallback = PlatformKeyCallback;
		callbacks.PlatformFocusCallback = PlatformFocusCallback;
		callbacks.PlatformMouseLeaveCallback = PlatformMouseLeaveCallback;
		callbacks.PlatformMouseScrollCallback = PlatformMouseScrollCallback;
		platform->functions.RegisterInputManager(mgr, &callbacks);
	}
	
	void InputBeginFrame(InputMgr* mgr) {
	}

	void InputEndFrame(InputMgr* mgr) {
		CopyArray(byte, KEYBOARD_KEYS_COUNT, mgr->keys->prev_state, mgr->keys->current_state);
		CopyArray(byte, MOUSE_BUTTONS_COUNT, mgr->mouse_buttons->prev_state, mgr->mouse_buttons->current_state);
		mgr->mouse_frame_offset_x = 0.0f;
		mgr->mouse_frame_offset_y = 0.0f;
		mgr->mouseFrameScrollOffset = 0;
	}

	int32 InputSubscribeEvent(InputMgr* mgr, const EventQuery* query) {
		InternalEventQuery* free_sub = nullptr;
		uint32 free_index = 0;
		for (uint32 i = 0; i < MAX_EVENT_SUBSC; i++) {
			if (mgr->subscriptions[i].handle == EVENT_INVALID_HANDLE) {
				free_sub = &mgr->subscriptions[i];
				free_index = i;
				break;
			}
		}
		if (free_sub) {
			if (free_index > mgr->last_subscription) {
				mgr->last_subscription = free_index;
			}
			free_sub->handle = mgr->sub_handle_counter;
			// TODO: Enable this assert
			//AB_CORE_ASSERT(mgr->sub_handle_counter < 0xfffffff, "Event handle out of range.");
			mgr->sub_handle_counter++;
			free_sub->query = *query;
			//mgr->subscriptions_at++;
			return free_sub->handle;
		} else {
			return EVENT_INVALID_HANDLE;
		}
	}

	void InputUnsubscribeEvent(InputMgr* mgr, int32 handle) {
		if (handle < mgr->sub_handle_counter) {
			for (uint32 i = 0; i < MAX_EVENT_SUBSC; i++) {
				if (mgr->subscriptions[i].handle == handle) {
					mgr->subscriptions[i].handle = EVENT_INVALID_HANDLE;
					if (i == mgr->last_subscription) {
						mgr->last_subscription--;
					}
					break;
				}
			}
		} else {
			// TODO: Enable this warning
			//AB_CORE_WARN("Unused or invalid handle passed.");
		}
	}

	hpm::Vector2 InputGetMousePosition(InputMgr* mgr) {
		return { mgr->mouse_pos_x, mgr->mouse_pos_y };
	}

	hpm::Vector2 InputGetMouseFrameOffset(InputMgr* mgr)
	{
		return {mgr->mouse_frame_offset_x, mgr->mouse_frame_offset_y};
	}

	void InputSetMouseMode(InputMgr* mgr, MouseMode mode) {
		mgr->mouse_mode= mode;
	}

	bool32 InputKeyIsPressed(InputMgr* mgr, KeyboardKey key) {
		return mgr->keys->current_state[(byte)key] && !mgr->keys->prev_state[(byte)key];
	}

	bool32 InputKeyIsDown(InputMgr* mgr, KeyboardKey key) {
		return mgr->keys->current_state[(byte)key];
	}

	bool32 InputKeyIsReleased(InputMgr* mgr, KeyboardKey key) {
		return !mgr->keys->current_state[(byte)key] && mgr->keys->prev_state[(byte)key];
	}

	bool32 InputMouseButtonIsPressed(InputMgr* mgr, MouseButton button) {
		return mgr->mouse_buttons->current_state[(byte)button] && !mgr->mouse_buttons->prev_state[(byte)button];
	}

	bool32 InputMouseButtonIsDown(InputMgr* mgr, MouseButton button) {
		return mgr->mouse_buttons->current_state[(byte)button];
	}

	bool32 InputMouseButtonIsReleased(InputMgr* mgr, MouseButton button) {
		return !mgr->mouse_buttons->current_state[(byte)button] && mgr->mouse_buttons->prev_state[(byte)button];
	}
}
