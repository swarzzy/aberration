#pragma once
#include "src/ABHeader.h"

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

		static constexpr uint64 VERTEX_BUFFER_SIZE = 4096;
		static constexpr uint64 INDEX_BUFFER_SIZE = 65536;

		static void Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY);
		static void Destroy();

		static uint32 LoadTexture(const char* filepath);
		static void DrawRectangle(hpm::Vector2 position, float32 angle, float32 anchor, hpm::Vector2 size, color32 color);

		static void Flush();

		friend void RenderGroupResizeCallback(uint32 width, uint32 height);
	};
}
