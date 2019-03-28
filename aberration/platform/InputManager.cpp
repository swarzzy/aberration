#include "InputManager.h"
#include "Application.h"
#include "Window.h"
#include "Memory.h"
#include "utils/Log.h"
#include <cstring>

namespace AB {

	static void DispatchEvent(InputMgr* mgr, const Event* e);

	void PlatformFocusCallback(bool32 focus) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
			mgr->window_active = focus;
		}
	}

	void PlatformKeyCallback(KeyboardKey key, bool32 state, uint16 sys_repeat_count) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
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
	}

	void PlatformMouseLeaveCallback(bool32 in_client_area) {
		auto* mgr = PermStorage()->input_manager;

		if (mgr) {
			mgr->mouse_in_client_area = in_client_area;
		}
	}

	void PlatformMouseButtonCallback(MouseButton button, bool32 state) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
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
	}

	void PlatformMouseCallback(uint32 x_pos, uint32 y_pos) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
			if (mgr->mouse_mode == MouseMode::Captured) {
				if (mgr->window_active) {
					uint32 win_w = 0;
					uint32 win_h = 0;
					WindowGetSize(&win_w, &win_h);
					int32 x_mid = win_w / 2;
					int32 y_mid = win_h / 2;
					float32 offset_x = (float32)((int32)x_pos - x_mid);
					float32 offset_y = (float32)((int32)y_pos - y_mid);
					mgr->mouse_pos_x += offset_x;
					mgr->mouse_pos_y += offset_y;

					Event e = {};
					e.type = EVENT_TYPE_MOUSE_MOVED;
					e.mouse_moved_event.mode = MouseMode::Captured;
					e.mouse_moved_event.x = offset_x;
					e.mouse_moved_event.y = offset_y;

					DispatchEvent(mgr, &e);
					WindowSetMousePosition(x_mid, y_mid);
				}
			} else if (mgr->mouse_mode == MouseMode::Cursor) {
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
		
	InputMgr* InputInitialize() {
		InputMgr** mgr = &GetMemory()->perm_storage.input_manager;
		if (!(*mgr)) {
			(*mgr) = (InputMgr*)SysAlloc(sizeof(InputMgr));

			// TODO: Make this data oriented and cache friendly
			for (uint32 i = 0; i < MAX_EVENT_SUBSC; i++) {
				(*mgr)->subscriptions[i].handle = EVENT_INVALID_HANDLE;
			}

			(*mgr)->window_active= WindowActive();
		}
		return (*mgr);
	}

	void InputBeginFrame(InputMgr* mgr) {
		WindowPollEvents();
	}

	void InputEndFrame(InputMgr* mgr) {
		CopyArray(byte, KEYBOARD_KEYS_COUNT, mgr->keys->prev_state, mgr->keys->current_state);
		CopyArray(byte, MOUSE_BUTTONS_COUNT, mgr->mouse_buttons->prev_state, mgr->mouse_buttons->current_state);
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
			AB_CORE_ASSERT(mgr->sub_handle_counter < 0xfffffff, "Event handle out of range.");
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
			AB_CORE_WARN("Unused or invalid handle passed.");
		}
	}

	hpm::Vector2 InputGetMousePosition(InputMgr* mgr) {
		return { mgr->mouse_pos_x, mgr->mouse_pos_y };
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
