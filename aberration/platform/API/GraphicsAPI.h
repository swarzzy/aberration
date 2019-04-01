#pragma once
#include "AB.h"
#include "utils/ImageLoader.h"

namespace AB::API
{
	enum class TextureFilter : uint32 {
		Linear = 0,
		Nearest
    };

	enum class TextureWrapMode : uint32 {
		ClampToEdge = 0,
		Repeat,
		MirroredRepeat
	};

	
	struct TextureParameters {
		TextureFilter filterMode;
		TextureWrapMode wrapMode;
	};
	
	AB_API uint32 CreateCubemap(TextureParameters params,
						 Image px, Image nx,
						 Image py, Image ny,
						 Image pz, Image nz); 
}
