#pragma once
#include "AB.h"
#include "utils/ImageLoader.h"

namespace hpm {
	union Vector2;
}

namespace AB {
	struct Rectangle;

	struct RendererDebugInfo {
		uint32 drawCalls;
		uint32 verticesDrawn;
	};

	constexpr uint64 RENDERER2D_DRAW_QUEUE_CAPACITY = 1024;
	constexpr uint64 RENDERER2D_INDEX_BUFFER_SIZE = RENDERER2D_DRAW_QUEUE_CAPACITY * 6;
	constexpr uint16 RENDERER2D_TEXTURE_STORAGE_CAPACITY = 256;
	constexpr uint16 RENDERER2D_FONT_STORAGE_SIZE = 1;
	constexpr uint64 RENDERER2D_FONT_MAX_CODEPOINTS = 500;
	constexpr uint16 RENDERER2D_DEFAULT_FONT_HANDLE = 1;
	constexpr float64 RENDERER2D_DEFAULT_MIN_DEPTH = 10;
	constexpr float64 RENDERER2D_DEFAULT_MAX_DEPTH = 0;

	void Renderer2DInitialize(uint32 drawableSpaceX, uint32 drawableSpaceY);
	void Renderer2DDestroy();
	uint32 Renderer2DGetDrawCallCount();
	hpm::Vector2 Renderer2DGetCanvasSize();

	hpm::Vector2 Renderer2DGetMousePositionOnCanvas();
	bool32 Renderer2DDrawRectangleColorUI(hpm::Vector2 min, hpm::Vector2 max, uint16 depth, float32 angle, float32 anchor, color32 color);
	uint16 Renderer2DLoadFont(const char* filepath);

	void Renderer2DDebugDrawString(hpm::Vector2 position, float32 height, color32 color, const char* string);
	void Renderer2DDebugDrawString(hpm::Vector2 position, float32 height, color32 color, const wchar_t* string);

	Rectangle Renderer2DGetStringBoundingRect(hpm::Vector2 position, float32 height, const char* string);
	Rectangle Renderer2DGetStringBoundingRect(hpm::Vector2 position, float32 height, const wchar_t* string);

	uint16 Renderer2DLoadTexture(const char* filepath);
	// TODO: Make enum for texture formats
	uint16 Renderer2DLoadTextureFromBitmap(API::TextureFormat format, uint32 width, uint32 height, const byte* bitmap);
	void Renderer2DFreeTexture(uint16 handle);
	uint16 Renderer2DTextureCreateRegion(uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
	// TODO: TextureDeleteRegion
	API::TextureFormat Renderer2DGetTextureFormat(uint16 handle);
	void Renderer2DFillRectangleColor(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color);
	void Renderer2DFillRectangleTexture(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle);

	void Renderer2DFlush();

	void RenderGroupResizeCallback(uint32 width, uint32 height);
};
    
