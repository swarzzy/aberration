#pragma once
#include "Types.h"
#include <hypermath.h>

namespace AB {
	struct DebugOverlayProperties {
		int64 frameTime;
		int64 fps;
		int64 ups;
		int32 drawCalls;
		hpm::Vector2 overlayBeginPos;
		float32 overlayAdvance;
		bool32 drawMainPane;
	};

	DebugOverlayProperties* CreateDebugOverlay();
	void DrawDebugOverlay(DebugOverlayProperties* properties);
	void DebugOverlayEnableMainPane(DebugOverlayProperties* properties, bool32 enable);
	void UpdateDebugOverlay(DebugOverlayProperties* properties);
	void DebugOverlayPushString(DebugOverlayProperties* properties, const char* string);
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector2 vec);
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector3 vec);
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector4 vec);
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, float32 vec);
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, int32 vec);
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, uint32 vec);




}
