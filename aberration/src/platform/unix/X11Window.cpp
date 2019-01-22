#include "../Window.h"
#include "src/utils/Log.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <GL/glx.h>

namespace AB {

	static constexpr uint32 AB_X11_MB_LEFT		= 1;
	static constexpr uint32 AB_X11_MB_RIGHT		= 3;
	static constexpr uint32 AB_X11_MB_MIDDLE	= 2;
	static constexpr uint32 AB_X11_MB_XB1		= 8;
	static constexpr uint32 AB_X11_MB_XB2		= 9;

	struct X11KeyState {
		bool currentState;
		bool prevState;
		uint32 repeatCount;
	};

	struct WindowProperties {
		String title;
		uint32 width;
		uint32 height;
		bool running;

		Display* X11Display;
		Screen* X11Screen;
		int32 X11ScreenID;
		int32 X11EventMask;
		::Window X11Window;
		Atom X11WmDeleteMessage;

		std::function<void()> closeCallback;
		std::function<void(uint32, uint32)> resizeCallback;

		int32 mousePositionX;
		int32 mousePositionY;
		std::function<void(MouseButton, bool)> mouseButtonCallback;
		std::function<void(uint32, uint32)> mouseMoveCallback;
		bool mouseButtonsCurrentState[MOUSE_BUTTONS_COUNT];
		bool mouseInClientArea;

		//bool gamepadCurrentState[GAMEPAD_STATE_ARRAY_SIZE];
		//bool gamepadPrevState[GAMEPAD_STATE_ARRAY_SIZE];
		//GamepadAnalogCtrl gamepadAnalogControls[XUSER_MAX_COUNT];
		//std::function<void(uint8, GamepadButton, bool, bool)> gamepadButtonCallback;
		//std::function<void(uint8, int16, int16, int16, int16)> gamepadStickCallback;
		//std::function<void(uint8, byte, byte)> gamepadTriggerCallback;

		X11KeyState keys[KEYBOARD_KEYS_COUNT]; // 1-current state, 2-prev state, 3+ - repeat count
		std::function<void(KeyboardKey, bool, bool, uint32)> keyCallback;
	};

	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint32 X11KeySym);

	Window::~Window() {

	}

	void Window::Create(const String& title, uint32 width, uint32 height) {
		if (s_WindowProperties) {
			AB_CORE_WARN("Window already initialized");
			return;
		}

		s_WindowProperties = ab_create WindowProperties{};
		s_WindowProperties->title = title;
		s_WindowProperties->width = width;
		s_WindowProperties->height = height;

		PlatformCreate();
	}

	void Window::Destroy() {
		// TODO: Temporary
		//glXDestroyContext(display, context);
		//XFree(vInfo);

		XUnmapWindow(s_WindowProperties->X11Display, s_WindowProperties->X11Window);
		XDestroyWindow(s_WindowProperties->X11Display, s_WindowProperties->X11Window);
		XCloseDisplay(s_WindowProperties->X11Display);
		ab_delete_scalar s_WindowProperties;
		s_WindowProperties = nullptr;
	}

	bool Window::IsOpen() {
		return s_WindowProperties->running;
	}

	void Window::PollEvents() {
		// NOTE: This behavior can be different than on windows. 
		// On windows we still processing events after running set to false
		if (s_WindowProperties->running) {
			XEvent event;
			Bool result = False;
			while (XPending(s_WindowProperties->X11Display)) {
				XNextEvent(s_WindowProperties->X11Display, &event);
				switch (event.type) {
					case ClientMessage: {
						if (event.xclient.data.l[0] == s_WindowProperties->X11WmDeleteMessage) {
							if (s_WindowProperties->closeCallback)
								s_WindowProperties->closeCallback();
							s_WindowProperties->running = false;
							XUnmapWindow(s_WindowProperties->X11Display, s_WindowProperties->X11Window);
						}
					} break;

					case KeymapNotify: {
						XRefreshKeyboardMapping(&event.xmapping);
					} break;

					case KeyPress: {
						char str[25];
						KeySym keysym;
						uint32 length = XLookupString(&event.xkey, str, 25, &keysym, NULL);
						uint8 key = _Win32KeyConvertToABKeycode(s_WindowProperties, keysym);

						s_WindowProperties->keys[key].prevState = s_WindowProperties->keys[key].currentState;
						s_WindowProperties->keys[key].currentState = true;

						KeyboardKey kbKey = static_cast<KeyboardKey>(key);
						// Check for Shift
						if (kbKey == KeyboardKey::LeftShift || kbKey == KeyboardKey::RightShift) {
							s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].prevState =
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].currentState;
							s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].currentState = true;
						}
						// Check for Ctrl
						if (kbKey == KeyboardKey::LeftCtrl || kbKey == KeyboardKey::RightCtrl) {
							s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].prevState =
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].currentState;
							s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].currentState = true;
						}

						// Callback for press and repeat events here
						if (s_WindowProperties->keyCallback)
							s_WindowProperties->keyCallback(static_cast<KeyboardKey>(key), true, s_WindowProperties->keys[key].prevState, 
															s_WindowProperties->keys[key].repeatCount);
					} break;

					case KeyRelease: {
						char str[25];
						KeySym keysym;
						uint32 length = XLookupString(&event.xkey, str, 25, &keysym, NULL);
						uint8 key = _Win32KeyConvertToABKeycode(s_WindowProperties, keysym);

						// Check if its actually repeat
						// Checking for repeats and increment repeatCounter in KeyRelease
						bool isRepeat = false;
						if (event.type == KeyRelease && XEventsQueued(s_WindowProperties->X11Display, QueuedAfterReading)) {
							XEvent nextEvent;
							XPeekEvent(s_WindowProperties->X11Display, &nextEvent);
							if (nextEvent.type == KeyPress && nextEvent.xkey.time == event.xkey.time &&
								nextEvent.xkey.keycode == event.xkey.keycode) {
								// This is repeat
								isRepeat = true;
								s_WindowProperties->keys[key].repeatCount++;
							}
						}

						KeyboardKey kbKey = static_cast<KeyboardKey>(key);
						// Check for Shift
						if (kbKey == KeyboardKey::LeftShift || kbKey == KeyboardKey::RightShift) {
							if (isRepeat) {
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].repeatCount++;
							}
							else {
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].prevState =
									s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].currentState;
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Shift)].currentState = false;
							}
						}

						// Check for Ctrl
						if (kbKey == KeyboardKey::LeftCtrl || kbKey == KeyboardKey::RightCtrl) {
							if (isRepeat) {
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].repeatCount++;
							}
							else {
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].prevState =
									s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].currentState;
								s_WindowProperties->keys[static_cast<uint8>(KeyboardKey::Ctrl)].currentState = false;
							}
						}

						if (!isRepeat) {
							s_WindowProperties->keys[key].prevState = s_WindowProperties->keys[key].currentState;
							s_WindowProperties->keys[key].currentState = false;

							// Callback for release event here
							if (s_WindowProperties->keyCallback)
								s_WindowProperties->keyCallback(kbKey, false, s_WindowProperties->keys[key].prevState, s_WindowProperties->keys[key].repeatCount);
							
							s_WindowProperties->keys[key].repeatCount = 0;
						}
					} break;

					case ButtonPress: {
						switch (event.xbutton.button) {
							case AB_X11_MB_LEFT: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Left)] = true;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::Left, true);
							} break;

							case AB_X11_MB_RIGHT: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Right)] = true;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::Right, true);
							} break;

							case AB_X11_MB_MIDDLE: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Middle)] = true;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::Middle, true);
							} break;

							case AB_X11_MB_XB1: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton1)] = true;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::XButton1, true);
							} break;

							case AB_X11_MB_XB2: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton2)] = true;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::XButton2, true);
							} break;
						}
					} break;

					case ButtonRelease: {
						switch (event.xbutton.button) {
							case AB_X11_MB_LEFT: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Left)] = false;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::Left, false);
							} break;

							case AB_X11_MB_RIGHT: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Right)] = false;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::Right, false);
							} break;

							case AB_X11_MB_MIDDLE: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::Middle)] = false;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::Middle, false);
							} break;

							case AB_X11_MB_XB1: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton1)] = false;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::XButton1, false);
							} break;

							case AB_X11_MB_XB2: {
								s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(MouseButton::XButton2)] = false;
								if (s_WindowProperties->mouseButtonCallback)
									s_WindowProperties->mouseButtonCallback(MouseButton::XButton2, false);
							} break;
						}
					} break;

					case MotionNotify: {
						s_WindowProperties->mousePositionX = event.xmotion.x;
						s_WindowProperties->mousePositionY = event.xmotion.y;
						if (s_WindowProperties->mouseMoveCallback)
							s_WindowProperties->mouseMoveCallback(event.xmotion.x, event.xmotion.y);

					} break;

					case EnterNotify: {
						s_WindowProperties->mouseInClientArea = true;
					} break;

					case LeaveNotify: {
						s_WindowProperties->mouseInClientArea = false;
					} break;

					case Expose: {
						XWindowAttributes attribs;
						XGetWindowAttributes(s_WindowProperties->X11Display, s_WindowProperties->X11Window, &attribs);
						s_WindowProperties->width = attribs.width;
						s_WindowProperties->height = attribs.height;
						if (s_WindowProperties->resizeCallback)
							s_WindowProperties->resizeCallback(attribs.width, attribs.height);
					} break;
				}


			}

		}
		// GL CONTEXT
		glViewport(0, 0, s_WindowProperties->width, s_WindowProperties->height);
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glXSwapBuffers(s_WindowProperties->X11Display, s_WindowProperties->X11Window);
	}


	void Window::GetSize(uint32& width, uint32& height) {
		width = s_WindowProperties->width;
		height = s_WindowProperties->height;
	}

	void Window::SetCloseCallback(const std::function<void()>& func) {
		s_WindowProperties->closeCallback = func;
	}

	void Window::SetResizeCallback(const std::function<void(uint32 width, uint32 height)>& func) {
		s_WindowProperties->resizeCallback = func;
	}

	bool Window::KeyPressed(KeyboardKey key) {
		return !(s_WindowProperties->keys[static_cast<uint8>(key)].prevState) && s_WindowProperties->keys[static_cast<uint8>(key)].currentState;
	}

	void Window::SetKeyCallback(const std::function<void(KeyboardKey key, bool currState, bool prevState, uint32 repeatCount)>& func) {
		s_WindowProperties->keyCallback = func;
	}

	void Window::GetMousePosition(uint32& xPos, uint32& yPos) {
		xPos = s_WindowProperties->mousePositionX;
		yPos = s_WindowProperties->mousePositionY;
	}

	bool Window::MouseButtonPressed(MouseButton button) {
		return s_WindowProperties->mouseButtonsCurrentState[static_cast<uint8>(button)];
	}

	bool Window::MouseInClientArea() {
		return s_WindowProperties->mouseInClientArea;
	}

	void Window::SetMouseButtonCallback(const std::function<void(MouseButton btn, bool state)>& func) {
		s_WindowProperties->mouseButtonCallback = func;
	}

	void Window::SetMouseMoveCallback(const std::function<void(uint32 xPos, uint32 yPos)>& func) {
		s_WindowProperties->mouseMoveCallback = func;
	}
	// TODO: implement gamepad input in linux
	// GAMEPAD INPUT (CURRENTLY NOT WORKING)

	bool Window::GamepadButtonPressed(uint8 gamepadNumber, GamepadButton button) {
		return false;
	}

	bool Window::GamepadButtonReleased(uint8 gamepadNumber, GamepadButton button) {
		return false;
	}

	bool Window::GamepadButtonHeld(uint8 gamepadNumber, GamepadButton button) {
		return false;
	}

	void Window::GetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY) {

	}

	void Window::GetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt) {

	}

	void Window::SetGamepadButtonCallback(const std::function<void(uint8 gpNumber, GamepadButton btn, bool currState, bool prevState)>& func) {
		
	}

	void Window::SetGamepadStickCallback(const std::function<void(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs)>& func) {
		
	}

	void Window::SetGamepadTriggerCallback(const std::function<void(uint8 gpNumber, byte lt, byte rt)>& func) {
		
	}

	// ^^^^ GAMEPAD INPUT

	void Window::PlatformCreate() {
		Display* display = XOpenDisplay(nullptr);
		AB_CORE_ASSERT(display, "Failed to open display");

		Screen* screen = DefaultScreenOfDisplay(display);
		AB_CORE_ASSERT(screen, "Failed to get screen");
		int32 screenID = DefaultScreen(display);

		// OpenGL context
		GLint majorGLX = 0;
		GLint minorGLX = 0;
		glXQueryVersion(display, &majorGLX, &minorGLX);
		if (majorGLX <= 1 && minorGLX <= 2) {
			AB_CORE_FATAL("Failed to create OpenGL context. GLX 1.2 of greater is required.");

		}
		AB_CORE_INFO("GLX version: ", majorGLX, ".", minorGLX);

		GLint glxAttribs[] = {
				GLX_RGBA,
				GLX_DOUBLEBUFFER, True,
				GLX_RED_SIZE, 8,
				 GLX_GREEN_SIZE, 8,
				GLX_BLUE_SIZE, 8,
				GLX_ALPHA_SIZE, 8,
				GLX_DEPTH_SIZE, 24,
				GLX_STENCIL_SIZE, 8,
				None
		};

		XVisualInfo* vInfo = glXChooseVisual(display, screenID, glxAttribs);
		AB_CORE_ASSERT(vInfo, "Failed to initialize OpenGL.");

		XSetWindowAttributes windowAttribs;
		windowAttribs.background_pixel = BlackPixel(display, screenID);
		windowAttribs.background_pixel = WhitePixel(display, screenID);
		windowAttribs.override_redirect = true;
		windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenID), vInfo->visual, AllocNone);
		windowAttribs.event_mask = ExposureMask;

		// ^^^^ GL CONTEXT

		s_WindowProperties->X11Window = XCreateWindow(
			display,
			RootWindow(display, screenID),
			0,
			0,
			s_WindowProperties->width,
			s_WindowProperties->height,
			0,
			vInfo->depth,
			InputOutput,
			vInfo->visual,
			CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
			&windowAttribs
		);

		XStoreName(display, s_WindowProperties->X11Window, s_WindowProperties->title.c_str());

		int32 eventMask = KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask |
							ButtonPressMask |ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | ExposureMask;
		XSelectInput(display, s_WindowProperties->X11Window, eventMask);

		// GL context

		GLXContext context = glXCreateContext(display, vInfo, NULL, GL_TRUE);
		glXMakeCurrent(display, s_WindowProperties->X11Window, context);

		AB_CORE_INFO("\nOpenGL Vendor: ", glGetString(GL_VENDOR),
			"\nOpenGL Renderer: ", glGetString(GL_RENDERER),
			"\nOpenGL Version: ", glGetString(GL_VERSION),
			"\nGLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

		// ^^^^ GL context

		XClearWindow(display, s_WindowProperties->X11Window);
		XMapRaised(display, s_WindowProperties->X11Window);

		// TODO: Error handling
		//auto wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
		s_WindowProperties->X11WmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, s_WindowProperties->X11Window, &s_WindowProperties->X11WmDeleteMessage, 1);

		s_WindowProperties->X11Display = display;
		s_WindowProperties->X11Screen = screen;
		s_WindowProperties->X11ScreenID= screenID;
		s_WindowProperties->X11EventMask = eventMask;
		s_WindowProperties->running = true;
	}

	// Does not return Shift and Ctrl keys
	// NOTE: Some symbols such as square bracker and curly bracket 
	// may be on different keys on some keyboards. 
	// AB layout currently assume that symbols like that always on the same key
	static uint8 _Win32KeyConvertToABKeycode(WindowProperties* window, uint32 X11KeySym) {

		switch(X11KeySym) {
			case XK_BackSpace:		return static_cast<uint8>(KeyboardKey::Backspace);
			case XK_Tab:			return static_cast<uint8>(KeyboardKey::Tab);
			case XK_Clear:			return static_cast<uint8>(KeyboardKey::Clear);
			case XK_Return:			return static_cast<uint8>(KeyboardKey::Enter);
			case XK_Alt_L:			return static_cast<uint8>(KeyboardKey::Alt);
			case XK_Alt_R:			return static_cast<uint8>(KeyboardKey::Alt);
			case XK_Pause:			return static_cast<uint8>(KeyboardKey::Pause);
			case XK_Caps_Lock:		return static_cast<uint8>(KeyboardKey::CapsLock);
			case XK_Escape:			return static_cast<uint8>(KeyboardKey::Escape);
				//TODO: check space
			case XK_space:			return static_cast<uint8>(KeyboardKey::Space);
			case XK_Page_Up:		return static_cast<uint8>(KeyboardKey::PageUp);
			case XK_Page_Down:		return static_cast<uint8>(KeyboardKey::PageDown);
			case XK_End:			return static_cast<uint8>(KeyboardKey::End);
			case XK_Home:			return static_cast<uint8>(KeyboardKey::Home);
			case XK_Left:			return static_cast<uint8>(KeyboardKey::Left);
			case XK_Right:			return static_cast<uint8>(KeyboardKey::Right);
			case XK_Up:				return static_cast<uint8>(KeyboardKey::Up);
			case XK_Down:			return static_cast<uint8>(KeyboardKey::Down);
				// check prtintsc
			case XK_Insert:			return static_cast<uint8>(KeyboardKey::Insert);
			case XK_Delete:			return static_cast<uint8>(KeyboardKey::Delete);
			case XK_0:				return static_cast<uint8>(KeyboardKey::Key0);
			case XK_1:				return static_cast<uint8>(KeyboardKey::Key1);
			case XK_2:				return static_cast<uint8>(KeyboardKey::Key2);
			case XK_3:				return static_cast<uint8>(KeyboardKey::Key3);
			case XK_4:				return static_cast<uint8>(KeyboardKey::Key4);
			case XK_5:				return static_cast<uint8>(KeyboardKey::Key5);
			case XK_6:				return static_cast<uint8>(KeyboardKey::Key6);
			case XK_7:				return static_cast<uint8>(KeyboardKey::Key7);
			case XK_8:				return static_cast<uint8>(KeyboardKey::Key8);
			case XK_9:				return static_cast<uint8>(KeyboardKey::Key9);
			case XK_A:				return static_cast<uint8>(KeyboardKey::A);
			case XK_B:				return static_cast<uint8>(KeyboardKey::B);
			case XK_C:				return static_cast<uint8>(KeyboardKey::C);
			case XK_D:				return static_cast<uint8>(KeyboardKey::D);
			case XK_E:				return static_cast<uint8>(KeyboardKey::E);
			case XK_F:				return static_cast<uint8>(KeyboardKey::F);
			case XK_G:				return static_cast<uint8>(KeyboardKey::G);
			case XK_H:				return static_cast<uint8>(KeyboardKey::H);
			case XK_I:				return static_cast<uint8>(KeyboardKey::I);
			case XK_J:				return static_cast<uint8>(KeyboardKey::J);
			case XK_K:				return static_cast<uint8>(KeyboardKey::K);
			case XK_L:				return static_cast<uint8>(KeyboardKey::L);
			case XK_M:				return static_cast<uint8>(KeyboardKey::M);
			case XK_N:				return static_cast<uint8>(KeyboardKey::N);
			case XK_O:				return static_cast<uint8>(KeyboardKey::O);
			case XK_P:				return static_cast<uint8>(KeyboardKey::P);
			case XK_Q:				return static_cast<uint8>(KeyboardKey::Q);
			case XK_R:				return static_cast<uint8>(KeyboardKey::R);
			case XK_S:				return static_cast<uint8>(KeyboardKey::S);
			case XK_T:				return static_cast<uint8>(KeyboardKey::T);
			case XK_U:				return static_cast<uint8>(KeyboardKey::U);
			case XK_V:				return static_cast<uint8>(KeyboardKey::V);
			case XK_W:				return static_cast<uint8>(KeyboardKey::W);
			case XK_X:				return static_cast<uint8>(KeyboardKey::X);
			case XK_Y:				return static_cast<uint8>(KeyboardKey::Y);
			case XK_Z:				return static_cast<uint8>(KeyboardKey::Z);
			case XK_a:				return static_cast<uint8>(KeyboardKey::A);
			case XK_b:				return static_cast<uint8>(KeyboardKey::B);
			case XK_c:				return static_cast<uint8>(KeyboardKey::C);
			case XK_d:				return static_cast<uint8>(KeyboardKey::D);
			case XK_e:				return static_cast<uint8>(KeyboardKey::E);
			case XK_f:				return static_cast<uint8>(KeyboardKey::F);
			case XK_g:				return static_cast<uint8>(KeyboardKey::G);
			case XK_h:				return static_cast<uint8>(KeyboardKey::H);
			case XK_i:				return static_cast<uint8>(KeyboardKey::I);
			case XK_j:				return static_cast<uint8>(KeyboardKey::J);
			case XK_k:				return static_cast<uint8>(KeyboardKey::K);
			case XK_l:				return static_cast<uint8>(KeyboardKey::L);
			case XK_m:				return static_cast<uint8>(KeyboardKey::M);
			case XK_n:				return static_cast<uint8>(KeyboardKey::N);
			case XK_o:				return static_cast<uint8>(KeyboardKey::O);
			case XK_p:				return static_cast<uint8>(KeyboardKey::P);
			case XK_q:				return static_cast<uint8>(KeyboardKey::Q);
			case XK_r:				return static_cast<uint8>(KeyboardKey::R);
			case XK_s:				return static_cast<uint8>(KeyboardKey::S);
			case XK_t:				return static_cast<uint8>(KeyboardKey::T);
			case XK_u:				return static_cast<uint8>(KeyboardKey::U);
			case XK_v:				return static_cast<uint8>(KeyboardKey::V);
			case XK_w:				return static_cast<uint8>(KeyboardKey::W);
			case XK_x:				return static_cast<uint8>(KeyboardKey::X);
			case XK_y:				return static_cast<uint8>(KeyboardKey::Y);
			case XK_z:				return static_cast<uint8>(KeyboardKey::Z);
			case XK_Super_L:		return static_cast<uint8>(KeyboardKey::LeftSuper);
			case XK_Super_R:		return static_cast<uint8>(KeyboardKey::RightSuper);
			// When numlock enabled
			case XK_KP_0:			return static_cast<uint8>(KeyboardKey::NumPad0);
			case XK_KP_1:			return static_cast<uint8>(KeyboardKey::NumPad1);
			case XK_KP_2:			return static_cast<uint8>(KeyboardKey::NumPad2);
			case XK_KP_3:			return static_cast<uint8>(KeyboardKey::NumPad3);
			case XK_KP_4:			return static_cast<uint8>(KeyboardKey::NumPad4);
			case XK_KP_5:			return static_cast<uint8>(KeyboardKey::NumPad5);
			case XK_KP_6:			return static_cast<uint8>(KeyboardKey::NumPad6);
			case XK_KP_7:			return static_cast<uint8>(KeyboardKey::NumPad7);
			case XK_KP_8:			return static_cast<uint8>(KeyboardKey::NumPad8);
			case XK_KP_9:			return static_cast<uint8>(KeyboardKey::NumPad9);
			case XK_KP_Decimal:		return static_cast<uint8>(KeyboardKey::NumPadDecimal);
			// When numlock disabled
			case XK_KP_Insert:		return static_cast<uint8>(KeyboardKey::NumPad0);
			case XK_KP_End:			return static_cast<uint8>(KeyboardKey::NumPad1);
			case XK_KP_Down:		return static_cast<uint8>(KeyboardKey::NumPad2);
			case XK_KP_Page_Down:	return static_cast<uint8>(KeyboardKey::NumPad3);
			case XK_KP_Left:		return static_cast<uint8>(KeyboardKey::NumPad4);
			case XK_KP_Begin:		return static_cast<uint8>(KeyboardKey::NumPad5);
			case XK_KP_Right:		return static_cast<uint8>(KeyboardKey::NumPad6);
			case XK_KP_Home:		return static_cast<uint8>(KeyboardKey::NumPad7);
			case XK_KP_Up:			return static_cast<uint8>(KeyboardKey::NumPad8);
			case XK_KP_Page_Up:		return static_cast<uint8>(KeyboardKey::NumPad9);
			case XK_KP_Delete:		return static_cast<uint8>(KeyboardKey::NumPadDecimal);
			case XK_KP_Enter:		return static_cast<uint8>(KeyboardKey::NumPadEnter);
			case XK_KP_Multiply:	return static_cast<uint8>(KeyboardKey::NumPadMultiply);
			case XK_KP_Add:			return static_cast<uint8>(KeyboardKey::NumPadAdd);
			case XK_KP_Subtract:	return static_cast<uint8>(KeyboardKey::NumPadSubtract);
			case XK_KP_Divide:		return static_cast<uint8>(KeyboardKey::NumPadDivide);
			case XK_F1:				return static_cast<uint8>(KeyboardKey::F1);
			case XK_F2:				return static_cast<uint8>(KeyboardKey::F2);
			case XK_F3:				return static_cast<uint8>(KeyboardKey::F3);
			case XK_F4:				return static_cast<uint8>(KeyboardKey::F4);
			case XK_F5:				return static_cast<uint8>(KeyboardKey::F5);
			case XK_F6:				return static_cast<uint8>(KeyboardKey::F6);
			case XK_F7:				return static_cast<uint8>(KeyboardKey::F7);
			case XK_F8:				return static_cast<uint8>(KeyboardKey::F8);
			case XK_F9:				return static_cast<uint8>(KeyboardKey::F9);
			case XK_F10:			return static_cast<uint8>(KeyboardKey::F10);
			case XK_F11:			return static_cast<uint8>(KeyboardKey::F11);
			case XK_F12:			return static_cast<uint8>(KeyboardKey::F12);
			case XK_F13:			return static_cast<uint8>(KeyboardKey::F13);
			case XK_F14:			return static_cast<uint8>(KeyboardKey::F14);
			case XK_F15:			return static_cast<uint8>(KeyboardKey::F15);
			case XK_F16:			return static_cast<uint8>(KeyboardKey::F16);
			case XK_F17:			return static_cast<uint8>(KeyboardKey::F17);
			case XK_F18:			return static_cast<uint8>(KeyboardKey::F18);
			case XK_F19:			return static_cast<uint8>(KeyboardKey::F19);
			case XK_F20:			return static_cast<uint8>(KeyboardKey::F20);
			case XK_F21:			return static_cast<uint8>(KeyboardKey::F21);
			case XK_F22:			return static_cast<uint8>(KeyboardKey::F22);
			case XK_F23:			return static_cast<uint8>(KeyboardKey::F23);
			case XK_F24:			return static_cast<uint8>(KeyboardKey::F24);
			case XK_Num_Lock:		return static_cast<uint8>(KeyboardKey::NumLock);
			case XK_Scroll_Lock:	return static_cast<uint8>(KeyboardKey::ScrollLock);
			case XK_Shift_L:		return static_cast<uint8>(KeyboardKey::LeftShift);
			case XK_Shift_R:		return static_cast<uint8>(KeyboardKey::RightShift);
			// Currently works only Ctrl for compability with Win32 implementation
			case XK_Control_L:		return static_cast<uint8>(KeyboardKey::Ctrl);
			case XK_Control_R:		return static_cast<uint8>(KeyboardKey::Ctrl);
			// left alt
			case XK_Menu:			return static_cast<uint8>(KeyboardKey::Menu);
			case XK_semicolon:		return static_cast<uint8>(KeyboardKey::Semicolon);
			case XK_equal:			return static_cast<uint8>(KeyboardKey::Equal);
			case XK_comma:			return static_cast<uint8>(KeyboardKey::Comma);
			case XK_minus:			return static_cast<uint8>(KeyboardKey::Minus);
			case XK_period:			return static_cast<uint8>(KeyboardKey::Period);
			case XK_slash:			return static_cast<uint8>(KeyboardKey::Slash);
			case XK_grave:			return static_cast<uint8>(KeyboardKey::Tilde);
			case XK_bracketleft:	return static_cast<uint8>(KeyboardKey::LeftBracket);
			case XK_bracketright:	return static_cast<uint8>(KeyboardKey::RightBracket);
			case XK_backslash:		return static_cast<uint8>(KeyboardKey::BackSlash);
			case XK_apostrophe:		return static_cast<uint8>(KeyboardKey::Apostrophe);
			case XK_Print:			return static_cast<uint8>(KeyboardKey::PrintScreen);
			default:				return static_cast<uint8>(KeyboardKey::InvalidKey);
		}
	}
}
