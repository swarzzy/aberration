#include "InputManager.h"
#include "Application.h"
#include "Window.h"
#include "Memory.h"

namespace AB {

	void PlatformFocusCallback(bool32 focus) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
			mgr->window_active = focus;
		}
	}

	void PlatformKeyCallback(KeyboardKey key, bool32 state, uint16 sys_repeat_count) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
			KeyInfo* key_info = &mgr->keys[(byte)key];

			key_info->prev_state = key_info->current_state;

			if (state) { // key down
				key_info->current_state = 1;
				key_info->sys_repeat_count += sys_repeat_count;
			}
			else {      // key up
				key_info->current_state = 0;
				key_info->sys_repeat_count = 0;
			}

			Event e = {};
			if (state && !key_info->prev_state) {
				e.type = EVENT_TYPE_KEY_PRESSED;
			}
			else if (state && key_info->prev_state) {
				e.type = EVENT_TYPE_KEY_REPEAT;
			}
			else if (!state) {
				e.type = EVENT_TYPE_KEY_RELEASED;
			}

			e.key_event.key = key;
			e.key_event.sys_repeat_count = sys_repeat_count;

			mgr->event_queue[mgr->event_queue_at] = e;
			mgr->event_queue_at++;
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
			MouseButtonInfo* info = &mgr->mouse_buttons[(byte)button];

			info->prev_state = info->current_state;
			info->current_state = state;

			Event e = {};
			if (state && !info->prev_state) {
				e.type = EVENT_TYPE_MOUSE_BTN_PRESSED;
			}
			else if (!state) {
				e.type = EVENT_TYPE_MOUSE_BTN_RELEASED;
			}

			e.mouse_button_event.button = button;

			mgr->event_queue[mgr->event_queue_at] = e;
			mgr->event_queue_at++;
		}
	}

	void PlatformMouseCallback(uint32 x_pos, uint32 y_pos) {
		auto* mgr = PermStorage()->input_manager;
		if (mgr) {
			if (mgr->mouse_mode == MouseMode::Captured) {
				if (mgr->window_active) {
					uint32 win_w = 0;
					uint32 win_h = 0;
					Window::GetSize(&win_w, &win_h);
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

					mgr->event_queue[mgr->event_queue_at] = e;
					mgr->event_queue_at++;

					Window::SetMousePosition(x_mid, y_mid);
				}
			} else if (mgr->mouse_mode == MouseMode::Cursor) {
				mgr->mouse_pos_x = (float32)x_pos;
				mgr->mouse_pos_y = (float32)y_pos;

				Event e = {};
				e.type = EVENT_TYPE_MOUSE_MOVED;
				e.mouse_moved_event.mode = MouseMode::Cursor;
				e.mouse_moved_event.x = (float32)x_pos;
				e.mouse_moved_event.y = (float32)y_pos;

				mgr->event_queue[mgr->event_queue_at] = e;
				mgr->event_queue_at++;
			}
		}
	}
		
	InputMgr* InputInitialize() {
		InputMgr** mgr = &GetMemory()->perm_storage.input_manager;
		if (!(*mgr)) {
			(*mgr) = (InputMgr*)SysAlloc(sizeof(InputMgr));
		}
		
		(*mgr)->window_active= Window::WindowActive();
		return (*mgr);
	}

	void InputSubscribeEvent(InputMgr* mgr, const EventQuery* query) {
		EventQuery* q = &mgr->subscriptions[mgr->subscriptions_at];
		*q = *query;
		mgr->subscriptions_at++;
	}

	hpm::Vector2 InputGetMousePosition(InputMgr* mgr) {
		return { mgr->mouse_pos_x, mgr->mouse_pos_y };
	}

	void InputUpdate(InputMgr* mgr) {
		for (uint32 e_index = 0; e_index < mgr->event_queue_at; e_index++) {
			Event* e = &mgr->event_queue[e_index];			
			for (uint32 q_index = 0; q_index < mgr->subscriptions_at; q_index++) {
				EventQuery* q = &mgr->subscriptions[q_index];
				if (e->type & q->type) {
					if (e->type == EVENT_TYPE_MOUSE_MOVED) {
						q->callback(*e);
						//if (!q->pass_through) {
						//	break;
						//}
					} else 
					if (e->type == EVENT_TYPE_KEY_PRESSED ||
						e->type == EVENT_TYPE_KEY_RELEASED ||
						e->type == EVENT_TYPE_KEY_REPEAT) {
						if (q->condition.key_event.key == e->key_event.key) {
							q->callback(*e);
							//if (!q->pass_through) {
							//	break;
							//}
						}
					} else
					if (e->type == EVENT_TYPE_MOUSE_BTN_PRESSED ||
						e->type == EVENT_TYPE_MOUSE_BTN_RELEASED) {
						if (q->condition.mouse_button_event.button == e->mouse_button_event.button) {
							q->callback(*e);
						}
					}
				}
			}
		}

		mgr->event_queue_at = 0;
	}

	void InputSetMouseMode(InputMgr* mgr, MouseMode mode) {
		mgr->mouse_mode= mode;
	}

	bool32 InputKeyIsDown(InputMgr* mgr, KeyboardKey key) {
		return mgr->keys[(byte)key].current_state;
	}

	bool32 InputMouseButtonIsDown(InputMgr* mgr, MouseButton button) {
		return mgr->mouse_buttons[(byte)button].current_state;
	}
}