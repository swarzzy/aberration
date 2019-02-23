#include "DebugTools.h"
#include "renderer/Renderer2D.h"
#include "platform/Platform.h"

namespace AB {

	void DrawDebugOverlay(DebugOverlayProperties* properties) {
		AB::Renderer2D::FillRectangleColor({ 20, 570 }, 8, 0, 0, { 430, 30 }, 0xee2e271e);
		bool32 pressed = AB::Renderer2D::DrawRectangleColorUI({ 0, 570 }, { 20, 30 }, 10, 0, 0, 0xff01a8ff);
		if (pressed) {
			AB::Renderer2D::FillRectangleColor({ 20, 570 }, 9, 0, 0, { 430, 30 }, 0xff03284f);
		}
		char buffer[64];
		AB::FormatString(buffer, 64, "%07.4f64 ms | %3i64 fps | %3i64 ups |%4u32 dc", properties->frameTime/ 1000.0, properties->fps, properties->ups, 0);
		hpm::Rectangle strr = AB::Renderer2D::GetStringBoundingRect({ 0,0 }, 20.0, buffer);
		float32 h = (strr.max.y - strr.min.y) / 2;
		AB::Renderer2D::DebugDrawString({ 35, 600 - h }, 20.0, 0xffffffff, buffer);
	}

	void UpdateDebugOverlay(DebugOverlayProperties* properties) {
		const ApplicationProperties* appProps = GetAppProperties();
		properties->frameTime = appProps->frameTime;
		properties->fps = appProps->fps;
		properties->ups = appProps->ups;
	}

}
