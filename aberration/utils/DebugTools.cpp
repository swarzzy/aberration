#include "DebugTools.h"
#include "render/DebugRenderer.h"
#include "platform/Common.h"
#include "platform/InputManager.h"
#include "platform/Window.h"
#include "Application.h"
#include "utils/Log.h"
#include "platform/Memory.h"
#include "ExtendedMath.h"

#include <cstring>

namespace AB {

	enum class DebugUIColors : uint32 {
		Pomegranate = 0xff2b39c0,
		Alizarin = 0xff3c4ce7,
		Midnightblue = 0xff503e2c,
		Clouds = 0xfff1f0ec
	};

	constexpr float32 DEBUG_OVERLAY_LINE_GAP = 10.0f;
	constexpr float32 DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP = 2.0f;

	DebugOverlayProperties* CreateDebugOverlay() {
		DebugOverlayProperties* properties = nullptr;
		properties = (DebugOverlayProperties*)SysAlloc(sizeof(DebugOverlayProperties));
		GetMemory()->perm_storage.debug_overlay = properties;
		auto canvasSize = Renderer2DGetCanvasSize();
		properties->overlayBeginPos = { 10, canvasSize.y - 40 };
		return properties;
	}
	static void _DebugOverlayDrawMainPane(DebugOverlayProperties* properties) {
		hpm::Vector2 canvas = Renderer2DGetCanvasSize();

		AB::Renderer2DFillRectangleColor({ 20, canvas.y - 30 }, 8, 0, 0, { 430, 30 }, (uint32)DebugUIColors::Midnightblue & 0xeeffffff);
		bool32 pressed = AB::Renderer2DDrawRectangleColorUI({ 0, canvas.y - 30 }, { 20, 30 }, 10, 0, 0, (uint32)DebugUIColors::Pomegranate);
		if (pressed) {
			AB::Renderer2DFillRectangleColor({ 20, canvas.y - 30 }, 9, 0, 0, { 430, 30 }, 0xff03284f);
		}
		char buffer[64];
		AB::FormatString(buffer, 64, "%07.4f64 ms | %3i64 fps | %3i64 ups |%4u32 dc", properties->frameTime / 1000.0, properties->fps, properties->ups, properties->drawCalls);
		Rectangle strr = AB::Renderer2DGetStringBoundingRect({ 0,0 }, 20.0, buffer);
		float32 h = (strr.max.y - strr.min.y) / 2;
		AB::Renderer2DDebugDrawString({ 35, canvas.y - h }, 20.0, (uint32)DebugUIColors::Clouds, buffer);
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
		properties->drawCalls = Renderer2DGetDrawCallCount();
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
		Renderer2DDebugDrawString(
			{ properties->overlayBeginPos.x, properties->overlayBeginPos.y - properties->overlayAdvance },
			20.0f,
			(uint32)DebugUIColors::Clouds,
			string
		);
		Rectangle bRect = Renderer2DGetStringBoundingRect({}, 20.0f, string);
		properties->overlayAdvance += hpm::Abs(bRect.min.y) + DEBUG_OVERLAY_LINE_GAP;
	}

	static void _DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, float32* val, float32 min, float32 max, float32 line_gap) {
		Renderer2DDebugDrawString(
			{ properties->overlayBeginPos.x, properties->overlayBeginPos.y - properties->overlayAdvance },
			20.0f,
			(uint32)DebugUIColors::Clouds,
			title
		);
		Rectangle bRect = Renderer2DGetStringBoundingRect({}, 20.0f, title);
		
		float32 sliderActualWidth = 250.0f;
		float32 sliderHeight = 20.0f;
		float32 blockWidth = 30.0f;
		float32 blockCenterOff = blockWidth / 2.0f;
		float32 sliderWidth = sliderActualWidth - blockCenterOff * 2.0f;

		float32 mappedVal = hpm::Map(*val, min, max, 0.0f, sliderWidth);

		Rectangle area;
		area.min.x = properties->overlayBeginPos.x + bRect.max.x + 5.0f;
		area.min.y = properties->overlayBeginPos.y - properties->overlayAdvance - sliderHeight;
		area.max.x = area.min.x + sliderActualWidth;
		area.max.y = area.min.y + sliderHeight;

		Rectangle block;
		block.min.x = area.min.x + mappedVal;
		block.min.y = area.min.y;
		block.max.x = block.min.x + blockWidth;
		block.max.y = block.min.y + sliderHeight;

		Renderer2DFillRectangleColor(
			area.min,
			8, // TODO: Make some const instead of this magic var
			0.0f,
			0.0f,
			hpm::Subtract(area.max, area.min),
			(uint32)DebugUIColors::Midnightblue & 0xeeffffff
		);

		Renderer2DFillRectangleColor(
			block.min,
			9, // TODO: Make some const instead of this magic var
			0.0f,
			0.0f,
			hpm::Subtract(block.max, block.min),
			(uint32)DebugUIColors::Pomegranate
		);

		if (InputMouseButtonIsDown(PermStorage()->input_manager, MouseButton::Left)) {
			hpm::Vector2 mousePos = Renderer2DGetMousePositionOnCanvas();

			if (Contains({ {area.min.x + blockCenterOff, area.min.y}, {area.max.x - blockCenterOff, area.max.y } }, { mousePos.x, mousePos.y })) {
				Renderer2DFillRectangleColor(
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

		properties->overlayAdvance += sliderHeight + line_gap;
	}

	void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, float32* val, float32 min, float32 max) {
		_DebugOverlayPushSlider(properties, title, val, min, max, DEBUG_OVERLAY_LINE_GAP);
	}

	void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, int32* val, int32 min, int32 max) {
		float32 fval = (float32)*val;
		_DebugOverlayPushSlider(properties, title, &fval, (float32)min, (float32)max + 0.1f, DEBUG_OVERLAY_LINE_GAP);
		*val = (int32)fval;
	}

	void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, uint32* val, uint32 min, uint32 max) {
		float32 fval = (float32)*val;
		_DebugOverlayPushSlider(properties, title, &fval, (float32)min, (float32)max + 0.1f, DEBUG_OVERLAY_LINE_GAP);
		*val = (uint32)fval;
	}

	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, Vector2* val, float32 min, float32 max) {
		_DebugOverlayPushSlider(properties, title, &val->x, min, max, DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(properties, title, &val->y, min, max, DEBUG_OVERLAY_LINE_GAP);
	}

	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, Vector3* val, float32 min, float32 max) {
		_DebugOverlayPushSlider(properties, title, &val->x, min, max, DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(properties, title, &val->y, min, max, DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(properties, title, &val->z, min, max, DEBUG_OVERLAY_LINE_GAP);

	}

	AB_API void DebugOverlayPushSlider(DebugOverlayProperties* properties, const char* title, Vector4* val, float32 min, float32 max) {
		_DebugOverlayPushSlider(properties, title, &val->x, min, max, DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(properties, title, &val->y, min, max, DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(properties, title, &val->z, min, max, DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(properties, title, &val->w, min, max, DEBUG_OVERLAY_LINE_GAP);
	}

	void DebugOverlayPushToggle(DebugOverlayProperties* properties, const char* title, bool32* val) {
		Renderer2DDebugDrawString({ properties->overlayBeginPos.x, properties->overlayBeginPos.y - properties->overlayAdvance },
								  20.0f,
								  (uint32)DebugUIColors::Clouds,
								  title
								  );
		Rectangle bRect = Renderer2DGetStringBoundingRect({}, 20.0f, title);
		
		float32 outerRectSize = 20.0f;
		float32 innerRectSize = 14.0f;
		float32 halfRectDiff = (outerRectSize - innerRectSize) / 2.0f;


		Rectangle outerRect;
		outerRect.min.x = properties->overlayBeginPos.x + bRect.max.x + 5.0f;
		outerRect.min.y = properties->overlayBeginPos.y - properties->overlayAdvance - outerRectSize;
		outerRect.max.x = outerRect.min.x + outerRectSize;
		outerRect.max.y = outerRect.min.y + outerRectSize;

		Rectangle innerRect;
		innerRect.min.x = outerRect.min.x + halfRectDiff;
		innerRect.min.y = outerRect.min.y + halfRectDiff;
		innerRect.max.x = innerRect.min.x + innerRectSize;
		innerRect.max.y = innerRect.min.y + innerRectSize;

		Renderer2DFillRectangleColor(
									 outerRect.min,
									 8, // TODO: Make some const instead of this magic var
									 0.0f,
									 0.0f,
									 hpm::Subtract(outerRect.max, outerRect.min),
									 (uint32)DebugUIColors::Midnightblue & 0xeeffffff
									 );

		if (*val) {
			Renderer2DFillRectangleColor(
										 innerRect.min,
										 9, // TODO: Make some const instead of this magic var
										 0.0f,
										 0.0f,
										 hpm::Subtract(innerRect.max, innerRect.min),
										 (uint32)DebugUIColors::Pomegranate
										 );

		}
		
		if (InputMouseButtonIsPressed(PermStorage()->input_manager, MouseButton::Left)) {
			hpm::Vector2 mousePos = Renderer2DGetMousePositionOnCanvas();
			if (Contains(outerRect, { mousePos.x, mousePos.y })) {
				*val = !(*val);
			}
		}

		properties->overlayAdvance += outerRectSize + DEBUG_OVERLAY_LINE_GAP;
		
	}

}
