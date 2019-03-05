#include "Application.h"
#include <cstdlib>
#include "utils/Log.h"
#include "platform/Window.h"
#include "renderer/Renderer2D.h"
#include "platform/Platform.h"
#include "platform/API/OpenGL/ABOpenGL.h"

namespace AB::Application {

	static constexpr int64 UPDATE_INTERVAL = 16000;
	static constexpr int64 SECOND_INTERVAL = 1000000;

	static Properties* g_App = nullptr;

	void Create() {
		if (!g_App) {
			g_App = (Properties*)malloc(sizeof(Properties));
			AB_CORE_ASSERT(g_App, "Failed to allocate Application.");
			memset(g_App, 0, sizeof(Properties));
		}
	}

	Properties* Get() {
		return g_App;
	}

	void Run() {
		AB_CORE_INFO("Aberration engine");

		Window::Create("Aberration", 1280, 720);
		Window::EnableVSync(true);

		Renderer2D::Initialize(1280, 720);

		g_App->running_time = AB::GetCurrentRawTime();

		g_App->debug_overlay = CreateDebugOverlay();
		AB::DebugOverlayEnableMainPane(g_App->debug_overlay, true);

		int64 update_timer = UPDATE_INTERVAL;
		int64 tick_timer = SECOND_INTERVAL;
		uint32 updates_since_last_tick = 0;

		if (g_App->init_callback) {
			g_App->init_callback();
		}

		while (AB::Window::IsOpen()) {
			if (tick_timer <= 0) {
				tick_timer = SECOND_INTERVAL;
				g_App->ups = updates_since_last_tick;
				updates_since_last_tick= 0;
				AB::UpdateDebugOverlay(g_App->debug_overlay);
			}

			if (update_timer<= 0) {
				update_timer = UPDATE_INTERVAL;
				updates_since_last_tick++;
				if (g_App->update_callback) {
					g_App->update_callback();
				}
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			AB::DrawDebugOverlay(g_App->debug_overlay);

			if (g_App->render_callback) {
				g_App->render_callback();
			}
			
			AB::Renderer2D::Flush();
			AB::Window::PollEvents();
			AB::Window::SwapBuffers();

			int64 current_time = AB::GetCurrentRawTime();
			g_App->frame_time = current_time - g_App->running_time;
			g_App->running_time = current_time;
			tick_timer -= g_App->frame_time;
			update_timer -= g_App->frame_time;
			g_App->fps = SECOND_INTERVAL / g_App->frame_time;
		}
	}

	void SetInitCallback(InitCallback* proc) {
		g_App->init_callback = proc;
	}

	void SetUpdateCallback(UpdateCallback* proc) {
		g_App->update_callback = proc;
	}

	void SetRenderCallback(RenderCallback* proc) {
		g_App->render_callback = proc;
	}
}