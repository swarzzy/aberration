#pragma once
#include "AB.h"
#include <hypermath.h>
#include "platform/Memory.h"

namespace AB {
	struct AB_API DebugOverlayProperties {
		int64 frameTime;
		int64 fps;
		int64 ups;
		int32 drawCalls;
		hpm::Vector2 overlayBeginPos;
		float32 overlayAdvance;
		bool32 drawMainPane;
	};

	DebugOverlayProperties* CreateDebugOverlay();
	AB_API void DrawDebugOverlay(DebugOverlayProperties* properties);
	AB_API void DebugOverlayEnableMainPane(DebugOverlayProperties* properties, bool32 enable);
	AB_API void UpdateDebugOverlay(DebugOverlayProperties* properties);
	AB_API void DebugOverlayPushString(DebugOverlayProperties* properties, const char* string);
	AB_API void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector2 vec);
	AB_API void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector3 vec);
	AB_API void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector4 vec);
	AB_API void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, float32 vec);
	AB_API void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, int32 vec);
	AB_API void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, uint32 vec);
	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, float32* val, float32 min, float32 max);
	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, int32* val, int32 min, int32 max);
	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, uint32* val, uint32 min, uint32 max);
	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, Vector2* val, float32 min, float32 max);
	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, Vector3* val, float32 min, float32 max);
	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, Vector4* val, float32 min, float32 max);



#define DEBUG_OVERLAY_PUSH_STR(str) AB::DebugOverlayPushString(AB::PermStorage()->debug_overlay, str)
#define DEBUG_OVERLAY_PUSH_VAR(title, var) AB::DebugOverlayPushVar(AB::PermStorage()->debug_overlay, title, var)

#define DEBUG_OVERLAY_PUSH_SLIDER(title, val, min, max) AB::DebugOverlayPushSlider(AB::PermStorage()->debug_overlay, title, val, min, max)
}
