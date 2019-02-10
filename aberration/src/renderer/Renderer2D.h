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
		static constexpr uint64 DRAW_QUEUE_CAPACITY = 16;
		static constexpr uint64 INDEX_BUFFER_SIZE = DRAW_QUEUE_CAPACITY * 6;
		static constexpr uint16 TEXTURE_STORAGE_CAPACITY = 128;

		static void Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY);
		static void Destroy();

		static uint16 LoadTexture(const char* filepath);
		static void FreeTexture(uint16 handle);
		static uint16 TextureCreateRegion(uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
		static PixelFormat GetTextureFormat(uint16 handle);
		static void FillRectangleColor(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color);
		static void FillRectangleTexture(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle);

		static void Flush();

		friend void RenderGroupResizeCallback(uint32 width, uint32 height);
	};
}
