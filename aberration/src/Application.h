#pragma once
#include "AB.h"
#include "utils/DebugTools.h"

namespace AB::Application {

	typedef void(InitCallback)();
	typedef void(UpdateCallback)();
	typedef void(RenderCallback)();
	
	struct AB_API Properties {
		DebugOverlayProperties* debug_overlay;
		InitCallback* init_callback;
		UpdateCallback* update_callback;
		RenderCallback* render_callback;
		int64 running_time;
		int64 frame_time;
		int64 fps;
		int64 ups;
	};

	AB_API void Create();
	AB_API Properties* Get();
	AB_API void SetInitCallback(InitCallback* proc);
	AB_API void SetUpdateCallback(UpdateCallback* proc);
	AB_API void SetRenderCallback(RenderCallback* proc);

	AB_API void Run();
}

#define AB_DECLARE_ENTRY_POINT(func) int main() { return func(); }