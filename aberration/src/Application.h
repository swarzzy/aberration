#pragma once
#include "AB.h"
#include "utils/DebugTools.h"

namespace AB{

	typedef void(InitCallback)();
	typedef void(UpdateCallback)();
	typedef void(RenderCallback)();

	struct AB_API Application {
		DebugOverlayProperties* debug_overlay;
		InitCallback* init_callback;
		UpdateCallback* update_callback;
		RenderCallback* render_callback;
		int64 running_time;
		int64 frame_time;
		int64 fps;
		int64 ups;
	};

	AB_API Application* AppCreate();
	AB_API void AppSetInitCallback(Application* app, InitCallback* proc);
	AB_API void AppSetUpdateCallback(Application* app, UpdateCallback* proc);
	AB_API void AppSetRenderCallback(Application* app, RenderCallback* proc);

	AB_API void AppRun(Application* app);
}

#define AB_DECLARE_ENTRY_POINT(func) int main() { return func(); }