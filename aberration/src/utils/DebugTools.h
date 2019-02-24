#pragma once
#include "Types.h"

namespace AB {
	struct DebugOverlayProperties {
		int64 frameTime;
		int64 fps;
		int64 ups;
		int32 drawCalls;
	};

	void DrawDebugOverlay(DebugOverlayProperties* properties);
	void UpdateDebugOverlay(DebugOverlayProperties* properties);
}