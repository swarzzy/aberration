#pragma once
#include "Shared.h"
#include <hypermath.h>
#include "Memory.h"
#include "render/DebugRenderer.h"

namespace AB
{
	
	struct DebugOverlay
	{
		DebugRenderer* renderer;
		i64 frameTime;
		i64 fps;
		i64 ups;
		v2 overlayBeginPos;
		f32 overlayAdvance;
		b32 drawMainPane;
	};

	DebugOverlay* CreateDebugOverlay(MemoryArena* memory,
									 DebugRenderer* renderer,
									 v2 renderCanvasSize);
	
	void DrawDebugOverlay(DebugOverlay* properties);
	
	void DebugOverlayEnableMainPane(DebugOverlay* overlay, b32 enable);
	void UpdateDebugOverlay(DebugOverlay* overlay, PlatformState* state);
	void DebugOverlayPushString(DebugOverlay* overlay, const char* string);
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, v2 vec);
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, v3 vec);
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, v4 vec);
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, f32 vec);
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, i32 vec);
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, u32 vec);
	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								f32* val, f32 min, f32 max);
	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								i32* val, i32 min, i32 max);
	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								u32* val, u32 min, u32 max);
	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								v2* val, f32 min, f32 max);
	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								v3* val, f32 min, f32 max);
	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								v4* val, f32 min, f32 max);
	void DebugOverlayPushToggle(DebugOverlay* overlay, const char* title,
								b32* val);

#define DEBUG_OVERLAY_PUSH_STR(str) DebugOverlayPushString(g_StaticStorage->debugOverlay, str)
#define DEBUG_OVERLAY_PUSH_VAR(title, var) DebugOverlayPushVar(g_StaticStorage->debugOverlay, title, var)
#define DEBUG_OVERLAY_PUSH_TOGGLE(title, varPtr) DebugOverlayPushToggle(g_StaticStorage->debugOverlay, title, varPtr)
#define DEBUG_OVERLAY_PUSH_SLIDER(title, val, min, max) DebugOverlayPushSlider(g_StaticStorage->debugOverlay, title, val, min, max)

#define DEBUG_OVERLAY_SLIDER(var, min, max) DebugOverlayPushSlider(g_StaticStorage->debugOverlay, #var, &var, min, max)
#define DEBUG_OVERLAY_TRACE_VAR(variableName)  DebugOverlayPushVar(g_StaticStorage->debugOverlay, #variableName, variableName)

}
