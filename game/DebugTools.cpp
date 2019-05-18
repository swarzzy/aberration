#include "DebugTools.h"
#include "render/DebugRenderer.h"
#include "InputManager.h"
#include "Log.h"
#include "Memory.h"
#include "ExtendedMath.h"

#include <cstring>

namespace AB
{
	
	enum class DebugUIColors : u32
	{
		Pomegranate = 0xff2b39c0,
		Alizarin = 0xff3c4ce7,
		Midnightblue = 0xff503e2c,
		Clouds = 0xfff1f0ec
	};

	constexpr f32 DEBUG_OVERLAY_LINE_GAP = 10.0f;
	constexpr f32 DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP = 2.0f;

	DebugOverlay* CreateDebugOverlay(MemoryArena* memory,
									 DebugRenderer* renderer,
									 v2 renderCanvasSize)
	{
		DebugOverlay* overlay = nullptr;
		overlay = (DebugOverlay*)PushSize(memory,
										  sizeof(DebugOverlay),
										  alignof(DebugOverlay));
		AB_CORE_ASSERT(overlay, "Allocation failed.");
		overlay->overlayBeginPos = { 10, renderCanvasSize.y - 40 };
		overlay->renderer = renderer;
		return overlay;
	}
	
	static void _DebugOverlayDrawMainPane(DebugOverlay* overlay)
	{
		v2 canvas = Renderer2DGetCanvasSize(overlay->renderer);
		u32 backColor = (u32)DebugUIColors::Midnightblue & 0xeeffffff;
		AB::Renderer2DFillRectangleColor(overlay->renderer, { 20, canvas.y - 30 },
										 8, 0, 0, { 430, 30 }, backColor);
		b32 pressed = AB::Renderer2DDrawRectangleColorUI(overlay->renderer,
														 { 0, canvas.y - 30 },
														 { 20, 30 }, 10, 0, 0,
														 (u32)DebugUIColors::Pomegranate);
		if (pressed)
		{
			AB::Renderer2DFillRectangleColor(overlay->renderer,
											 { 20, canvas.y - 30 },
											 9, 0, 0, { 430, 30 }, 0xff03284f);
		}
		char buffer[64];
		AB::FormatString(buffer, 64,
						 "%07.4f64 ms | %3i64 fps | %3i64 ups",
						 overlay->frameTime / 1000.0,
						 overlay->fps, overlay->ups);
		
		Rectangle strr = AB::Renderer2DGetStringBoundingRect(overlay->renderer,
															 { 0,0 },
															 20.0, buffer);
		f32 h = (strr.max.y - strr.min.y) / 2;
		AB::Renderer2DDebugDrawString(overlay->renderer,
									  { 35, canvas.y - h }, 20.0,
									  (u32)DebugUIColors::Clouds, buffer);
	}

	void DrawDebugOverlay(DebugOverlay* overlay)
	{
		if (overlay->drawMainPane)
		{
			_DebugOverlayDrawMainPane(overlay);
		}
		overlay->overlayAdvance = 0;
	}

	void DebugOverlayEnableMainPane(DebugOverlay* overlay,
									b32 enable)
	{
		overlay->drawMainPane = enable;
	}

	void UpdateDebugOverlay(DebugOverlay* overlay,
							PlatformState* state)
	{
		overlay->frameTime = state->frameTime;
		overlay->fps = state->fps;
		overlay->ups = state->ups;
	}

	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, v2 vec)
	{
		char buffer[128];
		FormatString(buffer, 128, "%s: %f32 %f32", title, vec.x, vec.y);
		DebugOverlayPushString(overlay, buffer);
	}

	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, v3 vec)
	{
		char buffer[128];
		FormatString(buffer, 128, "%s: %f32 %f32 %f32",
					 title, vec.x, vec.y, vec.z);
		DebugOverlayPushString(overlay, buffer);
	}

	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, v4 vec)
	{
		char buffer[128];
		FormatString(buffer, 128, "%s: %f32 %f32 %f32 %f32",
					 title, vec.x, vec.y, vec.z, vec.w);
		DebugOverlayPushString(overlay, buffer);
	}

	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, f32 var)
	{
		char buffer[128];
		FormatString(buffer, 128, "%s: %f32", title, var);
		DebugOverlayPushString(overlay, buffer);
	}

	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, i32 var)
	{
		char buffer[128];
		FormatString(buffer, 128, "%s: %i32", title, var);
		DebugOverlayPushString(overlay, buffer);
	}
	
	void DebugOverlayPushVar(DebugOverlay* overlay, const char* title, u32 var)
	{
		char buffer[128];
		FormatString(buffer, 128, "%s: %u32", title, var);
		DebugOverlayPushString(overlay, buffer);
	}

	void DebugOverlayPushString(DebugOverlay* overlay, const char* string)
	{
		v2 position = V2(overlay->overlayBeginPos.x,
						 overlay->overlayBeginPos.y - overlay->overlayAdvance);
		
		Renderer2DDebugDrawString(overlay->renderer,
								  position,	20.0f,
								  (uint32)DebugUIColors::Clouds, string);
		
		Rectangle bRect = Renderer2DGetStringBoundingRect(overlay->renderer,
														  {}, 20.0f, string);
		overlay->overlayAdvance += AbsF32(bRect.min.y) + DEBUG_OVERLAY_LINE_GAP;
	}

	static void _DebugOverlayPushSlider(DebugOverlay* overlay,
										const char* title,
										f32* val, f32 min, f32 max, f32 lineGap)
	{
		v2 titlePos = V2(overlay->overlayBeginPos.x,
						 overlay->overlayBeginPos.y - overlay->overlayAdvance);
		Renderer2DDebugDrawString(overlay->renderer, titlePos,
								  20.0f, (uint32)DebugUIColors::Clouds, title);
		
		Rectangle bRect = Renderer2DGetStringBoundingRect(overlay->renderer,
														  {}, 20.0f, title);
		
		f32 sliderActualWidth = 250.0f;
		f32 sliderHeight = 20.0f;
		f32 blockWidth = 30.0f;
		f32 blockCenterOff = blockWidth / 2.0f;
		f32 sliderWidth = sliderActualWidth - blockCenterOff * 2.0f;

		f32 mappedVal = Map(*val, min, max, 0.0f, sliderWidth);

		Rectangle area;
		area.min.x = overlay->overlayBeginPos.x + bRect.max.x + 5.0f;
		area.min.y = overlay->overlayBeginPos.y
			- overlay->overlayAdvance - sliderHeight;
		area.max.x = area.min.x + sliderActualWidth;
		area.max.y = area.min.y + sliderHeight;

		Rectangle block;
		block.min.x = area.min.x + mappedVal;
		block.min.y = area.min.y;
		block.max.x = block.min.x + blockWidth;
		block.max.y = block.min.y + sliderHeight;

		// TODO: Make some const instead of this magic var for depth
		Renderer2DFillRectangleColor(overlay->renderer,
									 area.min, 8, 0.0f,	0.0f,
									 SubV2V2(area.max, area.min),
									 (u32)DebugUIColors::Midnightblue & 0xeeffffff);
		// TODO: Make some const instead of this magic var
		Renderer2DFillRectangleColor(overlay->renderer,
									 block.min,	9, 0.0f, 0.0f,
									 SubV2V2(block.max, block.min),
									 (u32)DebugUIColors::Pomegranate);

		if (GlobalInput.mouseButtons[MBUTTON_LEFT].pressedNow)
		{
			v2 mousePos = Renderer2DGetMousePositionOnCanvas(overlay->renderer);

			Rectangle testRect = {{area.min.x + blockCenterOff, area.min.y},
								  {area.max.x - blockCenterOff, area.max.y}};
			b32 contains = Contains(testRect, { mousePos.x, mousePos.y });
			if (contains)
			{
				// TODO: Make some const instead of this magic var
				Renderer2DFillRectangleColor(overlay->renderer,block.min,
											 10, 0.0f, 0.0f,
											 SubV2V2(block.max, block.min),
											 (u32)DebugUIColors::Alizarin);

				f32 newVal = mousePos.x - area.min.x - blockCenterOff;
				f32 unmappedNewVal= hpm::Map(newVal, 0.0f, sliderWidth, min, max);
				*val = unmappedNewVal;
			}
		}

		overlay->overlayAdvance += sliderHeight + lineGap;
	}

	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								f32* val, f32 min, f32 max)
	{
		_DebugOverlayPushSlider(overlay, title, val, min,
								max, DEBUG_OVERLAY_LINE_GAP);
	}

	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								i32* val, i32 min, i32 max)
	{
		f32 fval = (f32)*val;
		_DebugOverlayPushSlider(overlay, title, &fval, (f32)min,
								(f32)max + 0.1f, DEBUG_OVERLAY_LINE_GAP);
		*val = (i32)fval;
	}

	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								u32* val, u32 min, u32 max)
	{
		f32 fval = (f32)*val;
		_DebugOverlayPushSlider(overlay, title, &fval, (f32)min,
								(f32)max + 0.1f, DEBUG_OVERLAY_LINE_GAP);
		*val = (u32)fval;
	}

	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								v2* val, f32 min, f32 max)
	{
		_DebugOverlayPushSlider(overlay, title, &val->x, min, max,
								DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(overlay, title, &val->y, min, max,
								DEBUG_OVERLAY_LINE_GAP);
	}

	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								v3* val, f32 min, f32 max)
	{
		_DebugOverlayPushSlider(overlay, title, &val->x, min, max,
								DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(overlay, title, &val->y, min, max,
								DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(overlay, title, &val->z, min, max,
								DEBUG_OVERLAY_LINE_GAP);
	}

	void DebugOverlayPushSlider(DebugOverlay* overlay, const char* title,
								v4* val, f32 min, f32 max)
	{
		_DebugOverlayPushSlider(overlay, title, &val->x, min, max,
								DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(overlay, title, &val->y, min, max,
								DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(overlay, title, &val->z, min, max,
								DEBUG_OVERLAY_SLIDER_VECTOR_LINE_GAP);
		_DebugOverlayPushSlider(overlay, title, &val->w, min, max,
								DEBUG_OVERLAY_LINE_GAP);
	}

	void DebugOverlayPushToggle(DebugOverlay* overlay, const char* title, b32* val)
	{
		v2 titlePos = {overlay->overlayBeginPos.x,
					   overlay->overlayBeginPos.y - overlay->overlayAdvance};
		Renderer2DDebugDrawString(overlay->renderer,
								  titlePos, 20.0f, (u32)DebugUIColors::Clouds, title);
		
		Rectangle bRect = Renderer2DGetStringBoundingRect(overlay->renderer,
														  {}, 20.0f, title);
		
		f32 outerRectSize = 20.0f;
		f32 innerRectSize = 14.0f;
		f32 halfRectDiff = (outerRectSize - innerRectSize) / 2.0f;

		Rectangle outerRect;
		outerRect.min.x = overlay->overlayBeginPos.x + bRect.max.x + 5.0f;
		outerRect.min.y = overlay->overlayBeginPos.y - overlay->overlayAdvance - outerRectSize;
		outerRect.max.x = outerRect.min.x + outerRectSize;
		outerRect.max.y = outerRect.min.y + outerRectSize;

		Rectangle innerRect;
		innerRect.min.x = outerRect.min.x + halfRectDiff;
		innerRect.min.y = outerRect.min.y + halfRectDiff;
		innerRect.max.x = innerRect.min.x + innerRectSize;
		innerRect.max.y = innerRect.min.y + innerRectSize;

		// TODO: Make some const instead of this magic var
		Renderer2DFillRectangleColor(overlay->renderer,
									 outerRect.min, 8, 0.0f, 0.0f,
									 hpm::SubV2V2(outerRect.max, outerRect.min),
									 (u32)DebugUIColors::Midnightblue & 0xeeffffff);

		if (*val)
		{
			// TODO: Make some const instead of this magic var
			Renderer2DFillRectangleColor(overlay->renderer,
										 innerRect.min, 9, 0.0f, 0.0f,
										 hpm::SubV2V2(innerRect.max, innerRect.min),
										 (u32)DebugUIColors::Pomegranate);
		}
		
		if (GlobalInput.mouseButtons[MBUTTON_LEFT].pressedNow)
		{
			v2 mousePos = Renderer2DGetMousePositionOnCanvas(overlay->renderer);
			if (Contains(outerRect, { mousePos.x, mousePos.y }))
			{
				*val = !(*val);
			}
		}
		overlay->overlayAdvance += outerRectSize + DEBUG_OVERLAY_LINE_GAP;
	}
}
