#pragma once
#include "AB.h"
#include "utils/ImageLoader.h"

namespace hpm {
	union Vector2;
	struct Rectangle;
}

namespace AB {
	struct RendererDebugInfo {
		uint32 drawCalls;
		uint32 verticesDrawn;
	};

	struct AB_API Renderer2D final {
		AB_DISALLOW_COPY_AND_MOVE(Renderer2D)
		inline static struct Renderer2DProperties* s_Properties = nullptr;

		Renderer2D() = delete;

		static constexpr uint64 DRAW_QUEUE_CAPACITY = 1024;
		static constexpr uint64 INDEX_BUFFER_SIZE = DRAW_QUEUE_CAPACITY * 6;
		static constexpr uint16 TEXTURE_STORAGE_CAPACITY = 256;
		static constexpr uint16 FONT_STORAGE_SIZE = 1;
		static constexpr uint64 FONT_MAX_CODEPOINTS = 500;
		static constexpr uint16 DEFAULT_FONT_HANDLE = 1;
		static constexpr float64 DEFAULT_MIN_DEPTH = 10;
		static constexpr float64 DEFAULT_MAX_DEPTH = 0;

		static void Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY);
		static void Destroy();
		static uint32 GetDrawCallCount();
		static hpm::Vector2 GetCanvasSize();

		static hpm::Vector2 GetMousePositionOnCanvas();
		static bool32 DrawRectangleColorUI(hpm::Vector2 min, hpm::Vector2 max, uint16 depth, float32 angle, float32 anchor, color32 color);
		static uint16 LoadFont(const char* filepath);

		static void DebugDrawString(hpm::Vector2 position, float32 height, color32 color, const char* string);
		static void DebugDrawString(hpm::Vector2 position, float32 height, color32 color, const wchar_t* string);

		static hpm::Rectangle GetStringBoundingRect(hpm::Vector2 position, float32 height, const char* string);
		static hpm::Rectangle GetStringBoundingRect(hpm::Vector2 position, float32 height, const wchar_t* string);

		static uint16 LoadTexture(const char* filepath);
		// TODO: Make enum for texture formats
		static uint16 LoadTextureFromBitmap(PixelFormat format, uint32 width, uint32 height, const byte* bitmap);
		static void FreeTexture(uint16 handle);
		static uint16 TextureCreateRegion(uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
		// TODO: TextureDeleteRegion
		static PixelFormat GetTextureFormat(uint16 handle);
		static void FillRectangleColor(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color);
		static void FillRectangleTexture(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle);

		static void Flush();

		friend void RenderGroupResizeCallback(uint32 width, uint32 height);
	};
}
