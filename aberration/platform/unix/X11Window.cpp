#include "../Window.h"
#include "utils/Log.h"
#include "../Memory.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include "../API/OpenGL/OpenGL.h"


// Declarations from <GL/glx.h>

typedef struct __GLXcontextRec *GLXContext;
typedef struct __GLXFBConfigRec *GLXFBConfig;
typedef XID GLXDrawable;

#define GLX_X_RENDERABLE		0x8012
#define GLX_DRAWABLE_TYPE		0x8010
#define GLX_WINDOW_BIT			0x00000001
#define GLX_X_VISUAL_TYPE		0x22
#define GLX_TRUE_COLOR			0x8002
#define GLX_RENDER_TYPE			0x8011
#define GLX_RGBA_BIT			0x00000001
#define GLX_DOUBLEBUFFER		5
#define GLX_RED_SIZE			8
#define GLX_GREEN_SIZE			9
#define GLX_BLUE_SIZE			10
#define GLX_ALPHA_SIZE			11
#define GLX_DEPTH_SIZE			12
#define GLX_STENCIL_SIZE		13

#define GLX_CONTEXT_MAJOR_VERSION_ARB				0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB				0x2092
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB			0x00000001
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 	0x00000002
#define GLX_CONTEXT_PROFILE_MASK_ARB     			0x9126

// GLX_ARB_multisample
#define GLX_SAMPLE_BUFFERS_ARB              		 100000
#define GLX_SAMPLES_ARB                      		 100001

extern "C" {
	extern void glXSwapBuffers( Display *dpy, GLXDrawable drawable );
	extern GLXDrawable glXGetCurrentDrawable( void );
	extern Bool glXQueryVersion( Display *dpy, int *maj, int *min );
	extern GLXFBConfig *glXChooseFBConfig( Display *dpy, int screen,
										   const int *attribList, int *nitems );
	extern XVisualInfo *glXGetVisualFromFBConfig( Display *dpy,
												  GLXFBConfig config );
	extern Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable,
								GLXContext ctx);
	extern const char *glXQueryExtensionsString( Display *dpy, int screen );

	extern void (*glXGetProcAddress(const GLubyte *procname))( void );
}

// TODO: Destroy context and delete all stuff in Destroy()

// FORWARD DECLARATIONS
namespace AB::GL {
	bool32 LoadFunctions();
	bool32 LoadExtensions();
	void InitAPI();
}

namespace AB {
	extern void RenderGroupResizeCallback(uint32 width, uint32 height);
	extern void PlatformMouseCallback(uint32 xPos, uint32 yPos);
	extern void PlatformMouseButtonCallback(MouseButton button, bool32 state);
	extern void PlatformKeyCallback(KeyboardKey key, bool32 state, uint16 sys_repeat_count);
	extern void PlatformFocusCallback(bool32 focus);
	extern void PlatformMouseLeaveCallback(bool32 in_client_area);
}

namespace AB {

	static constexpr uint32 AB_X11_MB_LEFT		= 1;
	static constexpr uint32 AB_X11_MB_RIGHT		= 3;
	static constexpr uint32 AB_X11_MB_MIDDLE	= 2;
	static constexpr uint32 AB_X11_MB_XB1		= 8;
	static constexpr uint32 AB_X11_MB_XB2		= 9;

	static constexpr uint32 OPENGL_MAJOR_VERSION = 4;
	static constexpr uint32 OPENGL_MINOR_VERSION = 5;

	// GLX Extensions

	typedef GLXContext proc_glXCreateContextAttribsARB(Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct,	const int *attrib_list);
	static proc_glXCreateContextAttribsARB* glXCreateContextAttribsARB = nullptr;

	typedef void proc_glXSwapIntervalEXT(Display *dpy, GLXDrawable drawable, int interval);
	static proc_glXSwapIntervalEXT* glXSwapIntervalEXT = nullptr;

	typedef int proc_glXSwapIntervalSGI(int interval);
	static proc_glXSwapIntervalSGI* glXSwapIntervalSGI = nullptr;

	// ^^^^ GLX Extensions

	struct X11KeyState {
		bool currentState;
		bool prevState;
		uint32 repeatCount;
	};

	static constexpr uint32 WINDOW_TITLE_SIZE = 32;

	struct WindowProperties {
		char title[32];
		uint32 width;
		uint32 height;
		bool32 running;
		//bool32 multisampling;
		//uint32 samples;

		bool32 activeWindow;

		Display* X11Display;
		Screen* X11Screen;
		int32 X11ScreenID;
		int32 X11EventMask;
		::Window X11Window;
		Atom X11WmDeleteMessage;

		PlatformCloseCallback* closeCallback;
		PlatformResizeCallback* resizeCallback;

		bool32 mouseInClientArea;

		X11KeyState keys[KEYBOARD_KEYS_COUNT];
	};

	static uint8 _X11KeyConvertToABKeycode(WindowProperties* window, uint32 X11KeySym);
	static bool32 _GLXLoadExtensions(WindowProperties* windowProps);
	static void _PlatformCreateWindowAndContext(WindowProperties* s_WindowProperties);

	void WindowCreate(const char* title, uint32 width, uint32 height) {
		if ((PermStorage()->window)) {
			AB_CORE_WARN("Window already initialized");
			return;
		}

		WindowProperties* properties = (WindowProperties*)SysAlloc(sizeof(WindowProperties));
		memcpy(properties->title, title, WINDOW_TITLE_SIZE);
		properties->width = width;
		properties->height = height;
		//properties->multisampling = multisampling;
		//properties->samples = samplesCount;
		
		GetMemory()->perm_storage.window = properties;
		_PlatformCreateWindowAndContext(properties);
	}

	void WindowDestroy() {
		AB_CORE_FATAL("Cannot destroy window for now. System allocator cannot free");
		auto window = PermStorage()->window;
		// TODO: Destroy Context
		//glXDestroyContext(display, context);
		//XFree(vInfo);

		XUnmapWindow(window->X11Display, window->X11Window);
		XDestroyWindow(window->X11Display, window->X11Window);
		XCloseDisplay(window->X11Display);
	}

	void WindowClose() {
		auto window = PermStorage()->window;
		window->running = false;
		XUnmapWindow(window->X11Display, window->X11Window);
	}

	bool WindowIsOpen() {
		auto window = PermStorage()->window;
		return window->running;
	}

	void WindowPollEvents() {
		auto window = PermStorage()->window;
		// NOTE: This behavior can be different than on windows. 
		// On windows we still processing events after running set to false
		if (window->running) {
			XEvent event;
			Bool result = False;
			while (XPending(window->X11Display)) {
				XNextEvent(window->X11Display, &event);
				switch (event.type) {
				case ClientMessage: {
					if (event.xclient.data.l[0] == window->X11WmDeleteMessage) {
						if (window->closeCallback)
							window->closeCallback();
						window->running = false;
						XUnmapWindow(window->X11Display, window->X11Window);
					}
				} break;

				case KeymapNotify: {
					XRefreshKeyboardMapping(&event.xmapping);
				} break;

				case KeyPress: {
					char str[25];
					KeySym keysym;
					uint32 length = XLookupString(&event.xkey, str, 25, &keysym, NULL);
					uint8 key = _X11KeyConvertToABKeycode(window, keysym);

					window->keys[key].prevState = window->keys[key].currentState;
					window->keys[key].currentState = true;
#if 0 // Why are we checking for it that way
					KeyboardKey kbKey = static_cast<KeyboardKey>(key);
					// Check for Shift
					if (kbKey == KeyboardKey::LeftShift || kbKey == KeyboardKey::RightShift) {
						window->keys[static_cast<uint8>(KeyboardKey::Shift)].prevState =
							window->keys[static_cast<uint8>(KeyboardKey::Shift)].currentState;
						window->keys[static_cast<uint8>(KeyboardKey::Shift)].currentState = true;
					}
					// Check for Ctrl
					if (kbKey == KeyboardKey::LeftCtrl || kbKey == KeyboardKey::RightCtrl) {
						window->keys[static_cast<uint8>(KeyboardKey::Ctrl)].prevState =
							window->keys[static_cast<uint8>(KeyboardKey::Ctrl)].currentState;
						window->keys[static_cast<uint8>(KeyboardKey::Ctrl)].currentState = true;
					}
#endif
					// Callback for press and repeat events here
					PlatformKeyCallback(static_cast<KeyboardKey>(key), true, window->keys[key].repeatCount);
				} break;

				case KeyRelease: {
					char str[25];
					KeySym keysym;
					uint32 length = XLookupString(&event.xkey, str, 25, &keysym, NULL);
					uint8 key = _X11KeyConvertToABKeycode(window, keysym);

					// Check if its actually repeat
					// Checking for repeats and increment repeatCounter in KeyRelease
					bool isRepeat = false;
					if (event.type == KeyRelease && XEventsQueued(window->X11Display, QueuedAfterReading)) {
						XEvent nextEvent;
						XPeekEvent(window->X11Display, &nextEvent);
						if (nextEvent.type == KeyPress && nextEvent.xkey.time == event.xkey.time &&
							nextEvent.xkey.keycode == event.xkey.keycode) {
							// This is repeat
							isRepeat = true;
							window->keys[key].repeatCount++;
						}
					}
#if 0
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
#endif
					if (!isRepeat) {
						window->keys[key].prevState = window->keys[key].currentState;
						window->keys[key].currentState = false;

						// Callback for release event here
						PlatformKeyCallback(static_cast<KeyboardKey>(key), false, window->keys[key].repeatCount);

						// TODO: Should repeat count be reset after callback in order to work as windows?
						window->keys[key].repeatCount = 0;
					}
				} break;

				case ButtonPress: {
					switch (event.xbutton.button) {
					case AB_X11_MB_LEFT: {
						PlatformMouseButtonCallback(MouseButton::Left, true);
					} break;
								
					case AB_X11_MB_RIGHT: {
						PlatformMouseButtonCallback(MouseButton::Right, true);
					} break;

					case AB_X11_MB_MIDDLE: {
						PlatformMouseButtonCallback(MouseButton::Middle, true);
					} break;

					case AB_X11_MB_XB1: {
						PlatformMouseButtonCallback(MouseButton::XButton1, true);
					} break;

					case AB_X11_MB_XB2: {
						PlatformMouseButtonCallback(MouseButton::XButton2, true);
					} break;
					}
				} break;

				case ButtonRelease: {
					switch (event.xbutton.button) {
					case AB_X11_MB_LEFT: {
						PlatformMouseButtonCallback(MouseButton::Left, false);
					} break;
								
					case AB_X11_MB_RIGHT: {
						PlatformMouseButtonCallback(MouseButton::Right, false);
					} break;

					case AB_X11_MB_MIDDLE: {
						PlatformMouseButtonCallback(MouseButton::Middle, false);
					} break;

					case AB_X11_MB_XB1: {
						PlatformMouseButtonCallback(MouseButton::XButton1, false);
					} break;

					case AB_X11_MB_XB2: {
						PlatformMouseButtonCallback(MouseButton::XButton2, false);
					} break;
					}
				} break;
						
				case MotionNotify: {
					PlatformMouseCallback(event.xmotion.x, window->height - event.xmotion.y);
				} break;

				case EnterNotify: {
					window->mouseInClientArea = true;
					PlatformMouseLeaveCallback(true);
				} break;

				case LeaveNotify: {
					window->mouseInClientArea = false;
					PlatformMouseLeaveCallback(false);
				} break;

				case Expose: {
					XWindowAttributes attribs;
					XGetWindowAttributes(window->X11Display, window->X11Window, &attribs);
					window->width = attribs.width;
					window->height = attribs.height;
					if (window->resizeCallback)
						window->resizeCallback(attribs.width, attribs.height);
					RenderGroupResizeCallback(attribs.width, attribs.height);
				} break;

				case FocusIn: {
					window->activeWindow = true;
					PlatformFocusCallback(true);
				}
					break;

				case FocusOut: {
					window->activeWindow = false;
					PlatformFocusCallback(false);
				}  break;
				}
			}
		}
	}
		
	

	void WindowSwapBuffers() {
		auto window = PermStorage()->window;
		glXSwapBuffers(window->X11Display, window->X11Window);
	}

	void WindowEnableVSync(bool32 enable) {
		auto window = PermStorage()->window;
		if (glXSwapIntervalEXT) {
			GLXDrawable drawable = glXGetCurrentDrawable();
			if (drawable) {
				if (enable)
					glXSwapIntervalEXT(window->X11Display, drawable, 1);
				else
					glXSwapIntervalEXT(window->X11Display, drawable, 0);
			}
		}
		else {
			if (enable)
				glXSwapIntervalSGI(1);
			else
				glXSwapIntervalSGI(0);
		}
	}

	void WindowGetSize(uint32* width, uint32* height) {
		auto window = PermStorage()->window;
		*width = window->width;
		*height = window->height;
	}

	void WindowSetCloseCallback(PlatformCloseCallback* func) {
		auto window = PermStorage()->window;
		window->closeCallback = func;
	}

	void WindowSetResizeCallback(PlatformResizeCallback* func) {
		auto window = PermStorage()->window;
		window->resizeCallback = func;
	}

	// TODO: Flip y?
	void WindowSetMousePosition(uint32 x, uint32 y) {
		auto window = PermStorage()->window;
		XWarpPointer(window->X11Display, None, window->X11Window, 0, 0, 0, 0, x, y);
		#if 0
		uint32 yFlipped = 0;
		if (y < s_WindowProperties->height) {
			yFlipped = s_WindowProperties->height - y;
			POINT pt = { (LONG)x, (LONG)yFlipped };
			if (ClientToScreen(s_WindowProperties->Win32WindowHandle, &pt)) {
				SetCursorPos(pt.x, pt.y);
			}
		}
		#endif 
	}

	void WindowShowCursor(bool32 show) {
		// TODO: Implement
	}

	bool32 WindowActive() {
		auto window = PermStorage()->window;
		return window->activeWindow;
	}

	// TODO: implement gamepad input in linux
	// GAMEPAD INPUT (CURRENTLY NOT WORKING)

	bool WindowGamepadButtonPressed(uint8 gamepadNumber, GamepadButton button) {
		return false;
	}

	bool WindowGamepadButtonReleased(uint8 gamepadNumber, GamepadButton button) {
		return false;
	}

	bool WindowGamepadButtonHeld(uint8 gamepadNumber, GamepadButton button) {
		return false;
	}

	void WindowGetGamepadStickPosition(uint8 gamepadNumber, int16& leftX, int16& leftY, int16& rightX, int16& rightY) {

	}

	void WindowGetGamepadTriggerPosition(uint8 gamepadNumber, byte& lt, byte& rt) {

	}

	void WindowSetGamepadButtonCallback(PlatformGamepadButtonCallback* func) {
		
	}

	void WindowSetGamepadStickCallback(PlatformGamepadStickCallback* func) {
		
	}

	void WindowSetGamepadTriggerCallback(PlatformGamepadTriggerCallback* func) {
		
	}

	// ^^^^ GAMEPAD INPUT

	static void _PlatformCreateWindowAndContext(WindowProperties* s_WindowProperties) {
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
		//AB_CORE_INFO("GLX version: ", majorGLX, ".", minorGLX);

		//int multisampling = s_WindowProperties->multisampling ? GLX_SAMPLE_BUFFERS_ARB : None;
		
		GLint glxAttribs[] = {
			GLX_X_RENDERABLE, True,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_DOUBLEBUFFER, True,
			GLX_RED_SIZE, 8,
			GLX_GREEN_SIZE, 8,
			GLX_BLUE_SIZE, 8,
			GLX_ALPHA_SIZE, 8,
			//GLX_DEPTH_SIZE, 24,
			//GLX_STENCIL_SIZE, 8,
			//multisampling, 1,
			//GLX_SAMPLES_ARB, SafeCastI32Int(s_WindowProperties->samples),
			None
		};

		int fbCount;
		GLXFBConfig* fbConfigs = glXChooseFBConfig(display, screenID, glxAttribs, &fbCount);
		AB_CORE_ASSERT(fbConfigs, "Failed to load OpenGL, Failed to get framebuffer config");

		// TODO: Choose best fbConfig manually
		// https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)
		//GLXFBConfig bestFBC = fbConfigs[1];
		//XFree(fbConfigs);

		XVisualInfo* vInfo = glXGetVisualFromFBConfig(display, fbConfigs[1]);
		//XVisualInfo* vInfo = glXChooseVisual(display, screenID, glxAttribs);
		AB_CORE_ASSERT(vInfo, "Failed to initialize OpenGL. Failed to get visual info");

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

		XStoreName(display, s_WindowProperties->X11Window, s_WindowProperties->title);

		int32 eventMask = KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask |
							ButtonPressMask |ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | ExposureMask;
		XSelectInput(display, s_WindowProperties->X11Window, eventMask);

		s_WindowProperties->X11Display = display;
		s_WindowProperties->X11Screen = screen;
		s_WindowProperties->X11ScreenID = screenID;
		s_WindowProperties->X11EventMask = eventMask;
		s_WindowProperties->running = true;

		// GL context

		bool32 GLXExtensionsLoaded = _GLXLoadExtensions(s_WindowProperties);
		AB_CORE_ASSERT(GLXExtensionsLoaded, "Failed to load GLX Extensions.");

		int contextAttribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, OPENGL_MAJOR_VERSION,
			GLX_CONTEXT_MINOR_VERSION_ARB, OPENGL_MINOR_VERSION,
			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		//GLXContext context = glXCreateContext(display, vInfo, NULL, GL_TRUE);
		GLXContext context = glXCreateContextAttribsARB(display, fbConfigs[1], 0, 1, contextAttribs);
		AB_CORE_ASSERT(context, "Failed to load OpenGL. Failed to create context.");
		glXMakeCurrent(display, s_WindowProperties->X11Window, context);

		//AB_CORE_INFO("\nOpenGL Vendor: ", glGetString(GL_VENDOR),
		//	"\nOpenGL Renderer: ", glGetString(GL_RENDERER),
		//	"\nOpenGL Version: ", glGetString(GL_VERSION),
		//	"\nGLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

		bool32 glLoadResult = GL::LoadFunctions();
		AB_CORE_ASSERT(glLoadResult, "Failed to load OpenGL");
		//bool32 glEXTLoadResult = GL::LoadExtensions();
		//AB_CORE_ASSERT(glLoadResult, "Failed to load OpenGL Extensions");
		GL::InitAPI();

		// ^^^^ GL context

		XClearWindow(display, s_WindowProperties->X11Window);
		XMapRaised(display, s_WindowProperties->X11Window);
		s_WindowProperties->activeWindow = true;

		// TODO: Error handling
		//auto wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
		s_WindowProperties->X11WmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, s_WindowProperties->X11Window, &s_WindowProperties->X11WmDeleteMessage, 1);
	}

	static bool32 _GLXLoadExtensions(WindowProperties* windowProps) {
		bool32 result = true;
		const char* glxExtensions = glXQueryExtensionsString(windowProps->X11Display, windowProps->X11ScreenID);
		if (!std::strstr(glxExtensions, "GLX_ARB_create_context_profile")) {
			AB_CORE_WARN("Failed to create OpenGL context. GLX_ARB_create_context_profile extension is not supported.");
			return 0;
		}
		if (!std::strstr(glxExtensions, "GLX_EXT_swap_control")) {
			// On some machines there is no GLX_EXT_swap_control but GLX_SGI_swap_control
			// However glXSwapIntervalEXT still gets loaded
			if (!std::strstr(glxExtensions, "GLX_SGI_swap_control")) {
				AB_CORE_WARN("Failed to create OpenGL context. GLX_EXT_swap_control extension is not supported.");
				return 0;
			}
		}

		const char* createContextProcName = "glXCreateContextAttribsARB";
		const char* swapIntervalProcNameEXT = "glXSwapIntervalEXT";
		const char* swapIntervalProcNameSGI = "glXSwapIntervalSGI";
		glXCreateContextAttribsARB = (proc_glXCreateContextAttribsARB*)glXGetProcAddress((const uchar*)createContextProcName);
		if (!glXCreateContextAttribsARB) {
			AB_CORE_WARN("Failed to create OpenGL context. Failed to load GLX_ARB_create_context_profile extension");
			result = false;
		}

		glXSwapIntervalEXT = (proc_glXSwapIntervalEXT*)glXGetProcAddress((const uchar*)swapIntervalProcNameEXT);
		if (!glXSwapIntervalEXT) {
			glXSwapIntervalSGI = (proc_glXSwapIntervalSGI*)glXGetProcAddress((const uchar*)swapIntervalProcNameSGI);
			if (glXSwapIntervalSGI) {
				AB_CORE_ASSERT(glXSwapIntervalSGI, "Failed to create OpenGL context. Failed to load GLX_SGI_swap_control extension");
				result = false;
			}
		}
		return result;
	}

	// Does not return Shift and Ctrl keys
	// NOTE: Some symbols such as square bracker and curly bracket 
	// may be on different keys on some keyboards. 
	// AB layout currently assume that symbols like that always on the same key
	static uint8 _X11KeyConvertToABKeycode(WindowProperties* window, uint32 X11KeySym) {

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
