#include "DebugTools.h"
#include "renderer/Renderer2D.h"
#include "platform/Platform.h"
#include "platform/InputManager.h"
#include "platform/Window.h"
#include "Application.h"
#include "utils/Log.h"
#include <cstring>
#include "platform/Memory.h"

namespace AB {

	enum class DebugUIColors : uint32 {
		Pomegranate = 0xff2b39c0,
		Alizarin = 0xff3c4ce7,
		Midnightblue = 0xff503e2c,
		Clouds = 0xfff1f0ec
	};

	constexpr float32 DEBUG_OVERLAY_LINE_GAP = 10.0f;

	DebugOverlayProperties* CreateDebugOverlay() {
		DebugOverlayProperties* properties = nullptr;
		properties = (DebugOverlayProperties*)SysAlloc(sizeof(DebugOverlayProperties));
		GetMemory()->perm_storage.debug_overlay = properties;
		auto canvasSize = Renderer2D::GetCanvasSize();
		properties->overlayBeginPos = { 10, canvasSize.y - 40 };
		return properties;
	}
	static void _DebugOverlayDrawMainPane(DebugOverlayProperties* properties) {
		hpm::Vector2 canvas = Renderer2D::GetCanvasSize();

		AB::Renderer2D::FillRectangleColor({ 20, canvas.y - 30 }, 8, 0, 0, { 430, 30 }, (uint32)DebugUIColors::Midnightblue & 0xeeffffff);
		bool32 pressed = AB::Renderer2D::DrawRectangleColorUI({ 0, canvas.y - 30 }, { 20, 30 }, 10, 0, 0, (uint32)DebugUIColors::Pomegranate);
		if (pressed) {
			AB::Renderer2D::FillRectangleColor({ 20, canvas.y - 30 }, 9, 0, 0, { 430, 30 }, 0xff03284f);
		}
		char buffer[64];
		AB::FormatString(buffer, 64, "%07.4f64 ms | %3i64 fps | %3i64 ups |%4u32 dc", properties->frameTime / 1000.0, properties->fps, properties->ups, properties->drawCalls);
		hpm::Rectangle strr = AB::Renderer2D::GetStringBoundingRect({ 0,0 }, 20.0, buffer);
		float32 h = (strr.max.y - strr.min.y) / 2;
		AB::Renderer2D::DebugDrawString({ 35, canvas.y - h }, 20.0, (uint32)DebugUIColors::Clouds, buffer);
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
		auto* app = PermStorage()->application;
		properties->frameTime = app->frame_time;
		properties->fps = app->fps;
		properties->ups = app->ups;
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
			(uint32)DebugUIColors::Clouds,
			string
		);
		hpm::Rectangle bRect = Renderer2D::GetStringBoundingRect({}, 20.0f, string);
		properties->overlayAdvance += hpm::Abs(bRect.min.y) + DEBUG_OVERLAY_LINE_GAP;
	}

	void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, float32* val, float32 min, float32 max) {
		Renderer2D::DebugDrawString(
			{ properties->overlayBeginPos.x, properties->overlayBeginPos.y - properties->overlayAdvance },
			20.0f,
			(uint32)DebugUIColors::Clouds,
			title
		);
		hpm::Rectangle bRect = Renderer2D::GetStringBoundingRect({}, 20.0f, title);
		
		float32 sliderActualWidth = 250.0f;
		float32 sliderHeight = 20.0f;
		float32 blockWidth = 30.0f;
		float32 blockCenterOff = blockWidth / 2.0f;
		float32 sliderWidth = sliderActualWidth - blockCenterOff * 2.0f;

		float32 mappedVal = hpm::Map(*val, min, max, 0.0f, sliderWidth);

		hpm::Rectangle area;
		area.min.x = properties->overlayBeginPos.x + bRect.max.x + 5.0f;
		area.min.y = properties->overlayBeginPos.y - properties->overlayAdvance - sliderHeight;
		area.max.x = area.min.x + sliderActualWidth;
		area.max.y = area.min.y + sliderHeight;

		hpm::Rectangle block;
		block.min.x = area.min.x + mappedVal;
		block.min.y = area.min.y;
		block.max.x = block.min.x + blockWidth;
		block.max.y = block.min.y + sliderHeight;

		Renderer2D::FillRectangleColor(
			area.min,
			8, // TODO: Make some const instead of this magic var
			0.0f,
			0.0f,
			hpm::Subtract(area.max, area.min),
			(uint32)DebugUIColors::Midnightblue & 0xeeffffff
		);

		Renderer2D::FillRectangleColor(
			block.min,
			9, // TODO: Make some const instead of this magic var
			0.0f,
			0.0f,
			hpm::Subtract(block.max, block.min),
			(uint32)DebugUIColors::Pomegranate
		);

		if (InputMouseButtonIsDown(PermStorage()->input_manager, MouseButton::Left)) {
			hpm::Vector2 mousePos = Renderer2D::GetMousePositionOnCanvas();

			if (hpm::Contains({ {area.min.x + blockCenterOff, area.min.y}, {area.max.x - blockCenterOff, area.max.y } }, { mousePos.x, mousePos.y })) {
				Renderer2D::FillRectangleColor(
					block.min,
					10, // TODO: Make some const instead of this magic var
					0.0f,
					0.0f,
					hpm::Subtract(block.max, block.min),
					(uint32)DebugUIColors::Alizarin
				);

				float32 newVal = mousePos.x - area.min.x - blockCenterOff;
				float32 unmappedNewVal= hpm::Map(newVal, 0.0f, sliderWidth, min, max);
				*val = unmappedNewVal;
			}
		}

		properties->overlayAdvance += sliderHeight + DEBUG_OVERLAY_LINE_GAP;
	}

	void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, int32* val, int32 min, int32 max) {
		float32 fval = (float32)*val;
		DebugOverlayPushSlider(properties, title, &fval, (float32)min, (float32)max + 0.1f);
		*val = (int32)fval;
	}

	void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, uint32* val, uint32 min, uint32 max) {
		float32 fval = (float32)*val;
		DebugOverlayPushSlider(properties, title, &fval, (float32)min, (float32)max + 0.1f);
		*val = (uint32)fval;
	}

}
