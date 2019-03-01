#include "DebugTools.h"
#include "renderer/Renderer2D.h"
#include "platform/Platform.h"
#include "../../../sandbox/src/InputManager.h"

namespace AB {

	DebugOverlayProperties* CreateDebugOverlay() {
		DebugOverlayProperties* properties = nullptr;
		properties = (DebugOverlayProperties*)malloc(sizeof(DebugOverlayProperties));
		memset(properties, 0, sizeof(DebugOverlayProperties));
		auto canvasSize = Renderer2D::GetCanvasSize();
		properties->overlayBeginPos = { 10, canvasSize.y - 40 };
		return properties;
	}
	static void _DebugOverlayDrawMainPane(DebugOverlayProperties* properties) {
		AB::Renderer2D::FillRectangleColor({ 20, 570 }, 8, 0, 0, { 430, 30 }, 0xee2e271e);
		bool32 pressed = AB::Renderer2D::DrawRectangleColorUI({ 0, 570 }, { 20, 30 }, 10, 0, 0, 0xff01a8ff);
		if (pressed) {
			AB::Renderer2D::FillRectangleColor({ 20, 570 }, 9, 0, 0, { 430, 30 }, 0xff03284f);
		}
		char buffer[64];
		AB::FormatString(buffer, 64, "%07.4f64 ms | %3i64 fps | %3i64 ups |%4u32 dc", properties->frameTime / 1000.0, properties->fps, properties->ups, properties->drawCalls);
		hpm::Rectangle strr = AB::Renderer2D::GetStringBoundingRect({ 0,0 }, 20.0, buffer);
		float32 h = (strr.max.y - strr.min.y) / 2;
		AB::Renderer2D::DebugDrawString({ 35, 600 - h }, 20.0, 0xffffffff, buffer);
	}

	void DrawDebugOverlay(DebugOverlayProperties* properties) {
		if (properties->drawMainPane) {
			_DebugOverlayDrawMainPane(properties);
		}
		properties->overlayAdvance = 0;
	}

	void DebugOverlayEnableMainPane(DebugOverlayProperties* properties, bool32 enable) {
		properties->drawMainPane = enable;
	}

	void UpdateDebugOverlay(DebugOverlayProperties* properties) {
		const ApplicationProperties* appProps = GetAppProperties();
		properties->frameTime = appProps->frameTime;
		properties->fps = appProps->fps;
		properties->ups = appProps->ups;
		properties->drawCalls = Renderer2D::GetDrawCallCount();
	}

	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector2 vec) {
		char buffer[128];
		AB::FormatString(buffer, 128, "%s: %f32 %f32", title, vec.x, vec.y);
		DebugOverlayPushString(properties, buffer);
	}

	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector3 vec) {
		char buffer[128];
		AB::FormatString(buffer, 128, "%s: %f32 %f32 %f32", title, vec.x, vec.y, vec.z);
		DebugOverlayPushString(properties, buffer);
	}

	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, hpm::Vector4 vec) {
		char buffer[128];
		AB::FormatString(buffer, 128, "%s: %f32 %f32 %f32 %f32", title, vec.x, vec.y, vec.z, vec.w);
		DebugOverlayPushString(properties, buffer);
	}

	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, float32 var) {
		char buffer[128];
		AB::FormatString(buffer, 128, "%s: %f32", title, var);
		DebugOverlayPushString(properties, buffer);
	}

	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, int32 var) {
		char buffer[128];
		AB::FormatString(buffer, 128, "%s: %i32", title, var);
		DebugOverlayPushString(properties, buffer);
	}
	void DebugOverlayPushVar(DebugOverlayProperties* properties, const char* title, uint32 var) {
		char buffer[128];
		AB::FormatString(buffer, 128, "%s: %u32", title, var);
		DebugOverlayPushString(properties, buffer);
	}

	void DebugOverlayPushString(DebugOverlayProperties* properties, const char* string) {
		Renderer2D::DebugDrawString(
			{ properties->overlayBeginPos.x, properties->overlayBeginPos.y - properties->overlayAdvance },
			20.0f,
			0xffffffff,
			string
		);
		hpm::Rectangle bRect = Renderer2D::GetStringBoundingRect({}, 20.0f, string);
		properties->overlayAdvance += hpm::Abs(bRect.min.y) + 10.0f;
	}

}
