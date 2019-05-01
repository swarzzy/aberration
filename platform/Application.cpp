#include "Application.h"
#include "Log.h"
#include "Window.h"
#include "Common.h"
#include "OpenGL.h"
#include "PlatformMemory.h"
#include "CodeLoader.h"

#include <cstdlib>

namespace AB {

	Application* g_Application = nullptr;

	inline static void _WindowRegisterInputManager(void* manager,
												   PlatformInputCallbacks* callbacks)
	{
		WindowRegisterInputManager(g_Application->window, manager, callbacks);
	}

	inline static void _WindowGetSize(u32* width, u32* height)
	{
		WindowGetSize(g_Application->window, width, height);
	}

	inline static void _WindowSetMousePosition(u32 x, u32 y)
	{
		WindowSetMousePosition(g_Application->window, x, y);
	}

	inline static b32 _WindowActive()
	{
		return WindowActive(g_Application->window);
	}

	static constexpr i64 UPDATE_INTERVAL = 16000;
	static constexpr i64 SECOND_INTERVAL = 1000000;

	Application* AppCreate(MemoryArena* sysMemory)
	{
		Application* app = nullptr;
		app = (Application*)PushSize(sysMemory, sizeof(Application), 0);
		AB_CORE_ASSERT(app, "Failed to allocate Application.");
		app->systemMemory = sysMemory;
		g_Application = app;
		return app;
	}

	void AppRun(Application* app)
	{
		AB_CORE_INFO("Aberration engine");
		app->state.runningTime = AB::GetCurrentRawTime();
		
		app->window = WindowAllocateAndInit(app->systemMemory,
											"Aberration", 1280, 720);
		WindowEnableVSync(app->window, true);

		LoadFunctionsResult glResult = OpenGLLoadFunctions(app->systemMemory);
		AB_CORE_ASSERT(glResult.success, "Failed to load OpenGL functions");
		InitOpenGL(glResult.funcTable);
		app->state.gl = glResult.funcTable;
		app->state.gl = glResult.funcTable;
		
		app->state.functions.RegisterInputManager = _WindowRegisterInputManager;
		app->state.functions.WindowGetSize = _WindowGetSize;
		app->state.functions.PlatformSetCorsorPosition = _WindowSetMousePosition;
		app->state.functions.WindowActive = _WindowActive;

		app->state.functions.ConsolePrint = ConsolePrint;
		app->state.functions.ConsoleSetColor = ConsoleSetColor;
		app->state.functions.DebugReadFilePermanent = DebugReadFilePermanent;
		app->state.functions.DebugGetFileSize = DebugGetFileSize;
		app->state.functions.DebugReadFile = DebugReadFileToBuffer;
		app->state.functions.DebugReadTextFile = DebugReadTextFileToBuffer;
		
		app->gameCode = AllocateGameCodeStruct(app->systemMemory);
		char execPath[256];
		SetupDirs(execPath, 256,
				  app->gameCode->libDir, MAX_GAME_LIB_PATH,
				  app->gameCode->libFullPath, MAX_GAME_LIB_PATH);
		
		b32 codeLoaded = UpdateGameCode(app->gameCode, app->window);
		AB_CORE_ASSERT(codeLoaded, "Failed to load code");

		app->gameMemory = AllocateArena(MEGABYTES(32));
		
		i64 updateTimer = UPDATE_INTERVAL;
		i64 tickTimer = SECOND_INTERVAL;
		u32 updatesSinceLastTick = 0;

		app->gameCode->GameInit(app->gameMemory, &app->state);

		while (AB::WindowIsOpen(app->window))
		{
			WindowPollEvents(app->window);

			if (tickTimer <= 0)
			{
				tickTimer = SECOND_INTERVAL;
				app->state.ups = updatesSinceLastTick;
				updatesSinceLastTick= 0;
			}

			if (updateTimer<= 0)
			{
				// TODO: temporary solution
				// Use callbacks instead
				WindowGetSize(app->window,
							  &app->state.windowWidth,
							  &app->state.windowHeight);
				app->state.windowActive = WindowActive(app->window);
				GetLocalTime(app->state.localTime);

				b32 codeReloaded = UpdateGameCode(app->gameCode, app->window);
				if (codeReloaded)
				{
					app->gameCode->GameReload(app->gameMemory, &app->state);				
				}
				updateTimer = UPDATE_INTERVAL;
				updatesSinceLastTick++;
				app->gameCode->GameUpdate(app->gameMemory, &app->state);
			}

			//app->state.gl->_glClearColor(1.0, 1.0, 0.0, 1.0);
			//app->state.gl->_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			app->gameCode->GameRender(app->gameMemory, &app->state);
			
			AB::WindowSwapBuffers(app->window);

			int64 current_time = AB::GetCurrentRawTime();
			app->state.frameTime = current_time - app->state.runningTime;
			app->state.runningTime = current_time;
			tickTimer -= app->state.frameTime;
			updateTimer -= app->state.frameTime;
			app->state.fps = SECOND_INTERVAL / app->state.frameTime;
		}
	}
}

int main()
{
	AB::CreateMemoryContext();
	AB::MemoryArena* sysArena = AB::AllocateArena(MEGABYTES(8));
	AB::Application* app = AB::AppCreate(sysArena);
	AB::AppRun(app);
	return 0;
}

