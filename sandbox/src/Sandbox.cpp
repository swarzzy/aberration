#include <Aberration.h>
#if defined(AB_PLATFORM_WINDOWS)
#include <conio.h>
#endif


#if defined(AB_PLATFORM_LINUX)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/glx.h>

void X11() {
	uint32 windowWidth = 320;
	uint32 windowHeight = 240;
	std::string windowTitle = "Aberration";

    Display* display = XOpenDisplay(NULL);
    AB_CORE_ASSERT(display, "Failed to open display.");

    Screen* screen = DefaultScreenOfDisplay(display);
    auto screenID = DefaultScreen(display);
    AB_CORE_ASSERT(screen, "Failed to create window");

    //Window window = XCreateSimpleWindow(
    //		display,
    //		RootWindowOfScreen(screen),
    //		0,
    //		0,
    //		windowWidth,
    //		windowHeight,
    //		1,
    //		BlackPixel(display, screenID),
    //		WhitePixel(display,screenID)
    //		);

	//XClearWindow(display, window);
	//XMapRaised(display, window);

    // GL context
    GLint majorGLX = 0;
    GLint minorGLX = 0;
    glXQueryVersion(display, &majorGLX, &minorGLX);
    if (majorGLX <=1 && minorGLX <= 2) {
        AB_CORE_FATAL("Failed to create OpenGL Context. GLX 1.2 or greater is required");
    }
    AB_CORE_INFO("GLX version: ", majorGLX, ".", minorGLX);

	// ^^^^ GL context

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

	printf("POINT\n");
    XVisualInfo* vInfo = glXChooseVisual(display, screenID, glxAttribs);
	printf("END POINT\n");
   // AB_CORE_ASSERT(vInfo, "Failed to initialize OpenGL.");

    XSetWindowAttributes windowAttribs;
    windowAttribs.background_pixel = BlackPixel(display, screenID);
    windowAttribs.background_pixel = WhitePixel(display, screenID);
    windowAttribs.override_redirect = true;
    windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenID), vInfo->visual, AllocNone);
    windowAttribs.event_mask = ExposureMask;

    Window window = XCreateWindow(
    		display,
    		RootWindow(display, screenID),
    		0,
    		0,
    		windowWidth,
    		windowHeight,
    		0,
    		vInfo->depth,
    		InputOutput,
    		vInfo->visual,
    		CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
    		&windowAttribs
    		);



	// GL context
	GLXContext context = glXCreateContext(display, vInfo, NULL, GL_TRUE);
	glXMakeCurrent(display, window, context);

	AB_CORE_INFO("\nOpenGL Vendor: ", glGetString(GL_VENDOR),
				 "\nOpenGL Renderer: ", glGetString(GL_RENDERER),
				 "\nOpenGL Version: ", glGetString(GL_VERSION),
				 "\nGLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

	XStoreName(display, window, windowTitle.c_str());

	int32 eventMask = KeyPressMask | KeyReleaseMask | KeymapStateMask | PointerMotionMask | ButtonPressMask |
						ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | ExposureMask;

	XSelectInput(
			display,
			window,
			eventMask
	);

    XClearWindow(display, window);
    XMapRaised(display, window);

    XEvent event;

    bool running = true;
    uint32 mouseX = 0;
    uint32 mouseY = 0;
    bool mouseCurrentState[3];
    bool mouseInClientArea = false;

    while (running) {
    	//XNextEvent(display, &event);
    	auto result = XCheckWindowEvent(display, window, eventMask, &event);
    	//if (result  == True)
    		//XNextEvent(display, &event);

    	if (result) {
			switch (event.type) {
				case KeymapNotify: {
					XRefreshKeyboardMapping(&event.xmapping);
				}
					break;

				case KeyPress: {
					char str[25];
					KeySym keysum;
					uint32 length = XLookupString(&event.xkey, str, 25, &keysum, NULL);
					if (keysum == XK_Escape)
						running = false;
					if (length > 0)
						printf("Key pressed: %s | %d | %lu\n", str, length, keysum);
				}
					break;

				case KeyRelease: {
					char str[25];
					KeySym keysum;
					uint32 length = XLookupString(&event.xkey, str, 25, &keysum, NULL);
					if (length > 0)
						printf("Key released: %s | %d | %lu\n", str, length, keysum);
				}
					break;

				case ButtonPress: {
					if (event.xbutton.button < 3)
						mouseCurrentState[event.xbutton.button] = true;
					printf("Mouse pressed: %d\n", event.xbutton.button);
				}
					break;

				case ButtonRelease: {
					if (event.xbutton.button < 3)
						mouseCurrentState[event.xbutton.button] = false;
					printf("Mouse released: %d\n", event.xbutton.button);
				}
					break;

				case MotionNotify: {
					mouseX = event.xmotion.x;
					mouseY = event.xmotion.y;
					printf("Mouse moved: %d %d\n", mouseX, mouseY);
				}
					break;

				case EnterNotify: {
					mouseInClientArea = true;
					printf("Mouse in client area\n");
				}
					break;

				case LeaveNotify: {
					mouseInClientArea = false;
					printf("Mouse leave client area\n");

				}
					break;

				case Expose: {
					XWindowAttributes attribs;
					XGetWindowAttributes(display, window, &attribs);
					windowWidth = attribs.width;
					windowHeight = attribs.height;
					printf("Window resized %d %d\n", windowWidth, windowHeight);
				}
					break;
					//case KeymapNotify: {

					//} break;
			}
		}
    	//event.type = 0;
		// OpenGl
		glViewport(0, 0, windowWidth, windowHeight);
    	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    	glClear(GL_COLOR_BUFFER_BIT);
    	glXSwapBuffers(display, window);
    	//printf("frame\n");

    }

    glXDestroyContext(display, context);

    XFree(vInfo);

    XDestroyWindow(display, window);
    XFree(screen);
    XCloseDisplay(display);
}
#endif

int main() {

	AB::utils::Log::Initialize(AB::utils::LogLevel::Info);
	AB::SystemMemoryInfo info = {};
	AB::GetSystemMemoryInfo(info);
	AB_CORE_INFO("Memory load: ", info.memoryLoad, "\nTotal phys: ", info.totalPhys / 1024 / 1024, "\nAvail phys: ", info.availablePhys / 1024 / 1024,
				 "\nTotal swap: ", info.totalSwap / (1024 * 1024), "\nAvail swap: ", info.availableSwap / 1024 / 1024,
				 "\n");
#if defined(AB_PLATFORM_LINUX)
	X11();
#elif defined(AB_PLATFORM_WINDOWS)

	AB::Window::Create("Aberration", 800, 600);
	AB::Window::SetResizeCallback([](uint32 w, uint32 h) {printf("Window resized: %u %u\n", w, h); });
	AB::Window::SetGamepadButtonCallback([](uint8 g, AB::GamepadButton b, bool c, bool p) {printf("Button: %d c: %d p: %d\n", b, c, p); });
	AB::Window::SetKeyCallback([](AB::KeyboardKey k, bool c, bool p, uint32 rc) {printf("Key pressed: %d %d %d %d\n", static_cast<uint32>(k), c, p, rc); });
	AB::Window::SetMouseButtonCallback([](AB::MouseButton b, bool s) {printf("Mouse pressed: %d %d\n", static_cast<uint8>(b), s); });
	//AB::Window::SetMouseMoveCallback([](uint32 x, uint32 y) {printf("%d %d\n", x, y); });
	
	while(AB::Window::IsOpen()) {
		
		AB::Window::PollEvents();
	}

	AB::Window::Destroy();


	//printf("lol");

#endif
	AB::AppMemoryInfo mem = {};
	AB::GetAppMemoryInfo(mem);
	AB_CORE_INFO("\nCurr: ", mem.currentUsed, "\nCurr alloc: ", mem.currentAllocations, "\nTotal: ", mem.totalUsed, "\nTotal alloc: ", mem.totalAllocations, "\n");
	std::vector<int, AB::Allocator<int>> list;

#if defined(AB_PLATFORM_WINDOWS)
	_getch();
#endif
	return 0;
}