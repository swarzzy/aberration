#include "Application.h"
#include <cstdlib>
#include "utils/Log.h"
#include "platform/Window.h"
#include "render/DebugRenderer.h"
#include "platform/Common.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Memory.h"

namespace AB {

	static constexpr int64 UPDATE_INTERVAL = 16000;
	static constexpr int64 SECOND_INTERVAL = 1000000;

	Application* AppCreate() {
		Application** app = &GetMemory()->perm_storage.application;
		if (!(*app)) {
			(*app) = (Application*)SysAlloc(sizeof(Application));
			AB_CORE_ASSERT((*app), "Failed to allocate Application.");
		}
		return (*app);
	}

	void AppRun(Application* app) {
		AB_CORE_INFO("Aberration engine");
		
		WindowCreate("Aberration", 1280, 720);
		WindowEnableVSync(true);
		

		Renderer2DInitialize(1280, 720);

		app->running_time = AB::GetCurrentRawTime();

		app->debug_overlay = CreateDebugOverlay();
		AB::DebugOverlayEnableMainPane(app->debug_overlay, true);

		int64 update_timer = UPDATE_INTERVAL;
		int64 tick_timer = SECOND_INTERVAL;
		uint32 updates_since_last_tick = 0;

		if (app->init_callback) {
			app->init_callback();
		}

		while (AB::WindowIsOpen()) {
			if (tick_timer <= 0) {
				tick_timer = SECOND_INTERVAL;
				app->ups = updates_since_last_tick;
				updates_since_last_tick= 0;
				AB::UpdateDebugOverlay(app->debug_overlay);
			}

			if (update_timer<= 0) {
				update_timer = UPDATE_INTERVAL;
				updates_since_last_tick++;
				if (app->update_callback) {
					app->update_callback();
				}
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			AB::DrawDebugOverlay(app->debug_overlay);

			if (app->render_callback) {
				app->render_callback();
			}
			
			AB::Renderer2DFlush();
			//AB::Window::PollEvents();
			AB::WindowSwapBuffers();

			int64 current_time = AB::GetCurrentRawTime();
			app->frame_time = current_time - app->running_time;
			app->running_time = current_time;
			tick_timer -= app->frame_time;
			update_timer -= app->frame_time;
			app->fps = SECOND_INTERVAL / app->frame_time;
		}
	}

	void AppSetInitCallback(Application* app, InitCallback* proc) {
		app->init_callback = proc;
	}

	void AppSetUpdateCallback(Application* app, UpdateCallback* proc) {
		app->update_callback = proc;
	}

	void AppSetRenderCallback(Application* app, RenderCallback* proc) {
		app->render_callback = proc;
	}
}
