#pragma once
#include "src/ABHeader.h"
#include "utils/ImageLoader.h"

namespace hpm {
	union Vector2;
}

namespace AB {

	class AB_API Renderer2D final {
		AB_DISALLOW_COPY_AND_MOVE(Renderer2D)
	private:
		inline static struct Renderer2DProperties* s_Properties = nullptr;

	public:
		Renderer2D() = delete;
		~Renderer2D() {};
		// TODO: Move tis into .cpp

		static constexpr uint64 DRAW_QUEUE_CAPACITY = 64;
		static constexpr uint64 INDEX_BUFFER_SIZE = DRAW_QUEUE_CAPACITY * 6;
		static constexpr uint16 TEXTURE_STORAGE_CAPACITY = 128;
		static constexpr uint16 FONT_STORAGE_SIZE = 1;
		static constexpr uint64 FONT_NUMBER_OF_CHARACTERS = 256;
		static constexpr uint16 DEFAULT_FONT_HANDLE = 1;

		static void Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY);
		static void Destroy();
		
		static uint16 LoadFont(const char* filepath);
		static void DebugDrawString(hpm::Vector2 position, float32 scale, const char* string);
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
