#pragma once
#include "AB.h"

namespace AB {
	struct Image;
}

namespace AB::API
{
	enum  TextureFilter : uint32 {
		TEX_FILTER_LINEAR = 0,
		TEX_FILTER_NEAREST
    };

	enum TextureWrapMode : uint32 {
		TEX_WRAP_CLAMP_TO_EDGE = 0,
		TEX_WRAP_REPEAT,
		TEX_WRAP_MIRR_REPEAT
	};

	enum  TextureFormat : uint32 {
		TEX_FORMAT_RED8 = 0,
		TEX_FORMAT_RGB8,
		TEX_FORMAT_RGBA8,
		TEX_FORMAT_RED16,
		TEX_FORMAT_RGB16,
	    TEX_FORMAT_RGBA16,
		TEX_FORMAT_SRGB8,
		TEX_FORMAT_SRGB8_A8,
		TEX_FORMAT_RGB16F,
		TEX_FORMAT_RGBA16F,
		TEX_FORMAT_DEPTH32
	};
	
	struct TextureParameters {
		TextureFilter filterMode;
		TextureWrapMode wrapMode;
	};

	enum ColorSpace : uint32 {
		TEX_COLOR_SPACE_LINEAR,
		TEX_COLOR_SPACE_SRGB
	};

	AB_API TextureFormat GetTextureFormat(uint32 bitsPerPixel, ColorSpace cs);

	AB_API uint32 CreateTexture(TextureParameters p, TextureFormat f,
								uint32 w, uint32 h, void* bitmap = nullptr);
	AB_API uint32 CreateTextureMultisample(TextureParameters p, TextureFormat f,
										   uint32 w, uint32 h, uint32 samples);
	
	AB_API uint32 CreateTexture(TextureParameters p, Image* img);

	AB_API uint32 CreateCubemap(TextureParameters params,
						 Image* px, Image* nx,
						 Image* py, Image* ny,
						 Image* pz, Image* nz);

	AB_API uint32 CreateFramebuffer();

	enum FramebufferClearFlags : uint32 {
		CLEAR_COLOR = 1 << 0,
		CLEAR_DEPTH = 1 << 1
	};
	
	AB_API void ClearCurrentFramebuffer(uint32 flags);

	enum  FramebufferTarget : uint32 {
		FB_TARGET_DRAW,
		FB_TARGET_READ,
		FB_TARGET_READ_DRAW
	};

	constexpr uint32 DEFAULT_FB_HANDLE = 0;
	
	AB_API void BindFramebuffer(uint32 handle, FramebufferTarget target);
	AB_API bool32 ValidateFramebuffer(FramebufferTarget type);

	enum FramebufferAttachment : uint32 {
		FB_ATTACHMENT_COLOR0,
		FB_ATTACHMENT_DEPTH
	};
	
	AB_API void FramebufferAttachTexture(FramebufferTarget target,
										 FramebufferAttachment att,
										 uint32 textureHandle);

	
	AB_API void FramebufferAttachTextureMultisample(FramebufferTarget target,
													FramebufferAttachment att,
													uint32 textureHandle);
}
