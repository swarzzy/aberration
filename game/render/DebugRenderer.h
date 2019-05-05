#pragma once
#include "AB.h"
#include "ImageLoader.h"

namespace hpm {
	union Vector2;
}

namespace AB {
	struct Rectangle;
	struct Renderer2DProperties;
	typedef Renderer2DProperties DebugRenderer;

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

	Renderer2DProperties* Renderer2DInitialize(MemoryArena* memoryArena,
											   MemoryArena* tempArena,
											   uint32 drawableSpaceX,
											   uint32 drawableSpaceY);
	void Renderer2DDestroy(Renderer2DProperties* renderer);
	uint32 Renderer2DGetDrawCallCount(Renderer2DProperties* renderer);
	hpm::Vector2 Renderer2DGetCanvasSize(Renderer2DProperties* renderer);

	hpm::Vector2 Renderer2DGetMousePositionOnCanvas(Renderer2DProperties* renderer,
													InputMgr* inputManager);
	bool32 Renderer2DDrawRectangleColorUI(Renderer2DProperties* renderer,
										  InputMgr* inputManager,
										  hpm::Vector2 min, hpm::Vector2 max,
										  uint16 depth, float32 angle,
										  float32 anchor, color32 color);
	uint16 Renderer2DLoadFont(MemoryArena* memoryArena, Renderer2DProperties* renderer, const char* filepath);

	void Renderer2DDebugDrawString(Renderer2DProperties* renderer,
								   hpm::Vector2 position, float32 height,
								   color32 color, const char* string);
	void Renderer2DDebugDrawString(Renderer2DProperties* renderer,
								   hpm::Vector2 position, float32 height,
								   color32 color, const wchar_t* string);

	Rectangle Renderer2DGetStringBoundingRect(Renderer2DProperties* renderer,
											  hpm::Vector2 position, float32 height,
											  const char* string);
	Rectangle Renderer2DGetStringBoundingRect(Renderer2DProperties* renderer, hpm::Vector2 position,
											  float32 height, const wchar_t* string);

	uint16 Renderer2DLoadTexture(MemoryArena* memory,
								 Renderer2DProperties* renderer,
								 const char* filepath);
	// TODO: Make enum for texture formats
	uint16 Renderer2DLoadTextureFromBitmap(Renderer2DProperties* renderer,
										   TextureFormat format,
										   uint32 width, uint32 height,
										   const byte* bitmap);
	// TODO: Use stack allocator
	//void Renderer2DFreeTexture(Renderer2DProperties* renderer, uint16 handle);
	uint16 Renderer2DTextureCreateRegion(Renderer2DProperties* renderer,
										 uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
	// TODO: TextureDeleteRegion
	TextureFormat Renderer2DGetTextureFormat(Renderer2DProperties* renderer,
											 uint16 handle);
	void Renderer2DFillRectangleColor(Renderer2DProperties* renderer, hpm::Vector2 position, uint16 depth,
									  float32 angle, float32 anchor, hpm::Vector2 size, color32 color);
	void Renderer2DFillRectangleTexture(Renderer2DProperties* renderer, hpm::Vector2 position, uint16 depth,
										float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle);

	void Renderer2DFlush(Renderer2DProperties* renderer, PlatformState* platform);

	void RenderGroupResizeCallback(Renderer2DProperties* renderer, uint32 width, uint32 height);
};
    
