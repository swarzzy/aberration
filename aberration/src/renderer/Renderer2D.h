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

	namespace Renderer2D {
		constexpr uint64 DRAW_QUEUE_CAPACITY = 1024;
		constexpr uint64 INDEX_BUFFER_SIZE = DRAW_QUEUE_CAPACITY * 6;
		constexpr uint16 TEXTURE_STORAGE_CAPACITY = 256;
		constexpr uint16 FONT_STORAGE_SIZE = 1;
		constexpr uint64 FONT_MAX_CODEPOINTS = 500;
		constexpr uint16 DEFAULT_FONT_HANDLE = 1;
		constexpr float64 DEFAULT_MIN_DEPTH = 10;
		constexpr float64 DEFAULT_MAX_DEPTH = 0;

		void Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY);
		void Destroy();
		uint32 GetDrawCallCount();
		hpm::Vector2 GetCanvasSize();

		hpm::Vector2 GetMousePositionOnCanvas();
		bool32 DrawRectangleColorUI(hpm::Vector2 min, hpm::Vector2 max, uint16 depth, float32 angle, float32 anchor, color32 color);
		uint16 LoadFont(const char* filepath);

		void DebugDrawString(hpm::Vector2 position, float32 height, color32 color, const char* string);
		void DebugDrawString(hpm::Vector2 position, float32 height, color32 color, const wchar_t* string);

		hpm::Rectangle GetStringBoundingRect(hpm::Vector2 position, float32 height, const char* string);
		hpm::Rectangle GetStringBoundingRect(hpm::Vector2 position, float32 height, const wchar_t* string);

		uint16 LoadTexture(const char* filepath);
		// TODO: Make enum for texture formats
		uint16 LoadTextureFromBitmap(PixelFormat format, uint32 width, uint32 height, const byte* bitmap);
		void FreeTexture(uint16 handle);
		uint16 TextureCreateRegion(uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
		// TODO: TextureDeleteRegion
		PixelFormat GetTextureFormat(uint16 handle);
		void FillRectangleColor(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color);
		void FillRectangleTexture(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle);

		void Flush();

		void RenderGroupResizeCallback(uint32 width, uint32 height);
	};
}
