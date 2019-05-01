#pragma once
#include "AB.h"

namespace AB
{
	struct Image;

	enum  TextureFilter : u32
	{
		TEX_FILTER_LINEAR = 0,
		TEX_FILTER_NEAREST
    };

	enum TextureWrapMode : u32
	{
		TEX_WRAP_CLAMP_TO_EDGE = 0,
		TEX_WRAP_REPEAT,
		TEX_WRAP_MIRR_REPEAT
	};

	enum  TextureFormat : u32
	{
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
	
	struct TextureParameters
	{
		TextureFilter filterMode;
		TextureWrapMode wrapMode;
	};

	enum ColorSpace : u32
	{
		TEX_COLOR_SPACE_LINEAR,
		TEX_COLOR_SPACE_SRGB
	};

	TextureFormat GetTextureFormat(u32 bitsPerPixel, ColorSpace cs);

	u32 CreateTexture(TextureParameters p, TextureFormat f,
					  u32 w, u32 h, void* bitmap = nullptr);

	u32 CreateTextureMultisample(TextureParameters p, TextureFormat f,
								 u32 w, u32 h, u32 samples);
	
	u32 CreateTexture(TextureParameters p, Image* img);

	u32 CreateCubemap(TextureParameters params,
					  Image* px, Image* nx,
					  Image* py, Image* ny,
					  Image* pz, Image* nz);

	u32 CreateFramebuffer();

	enum FramebufferClearFlags : u32
	{
		CLEAR_COLOR = 1 << 0,
		CLEAR_DEPTH = 1 << 1
	};
	
	void ClearCurrentFramebuffer(u32 flags);

	enum  FramebufferTarget : u32
	{
		FB_TARGET_DRAW,
		FB_TARGET_READ,
		FB_TARGET_READ_DRAW
	};

	constexpr u32 DEFAULT_FB_HANDLE = 0;
	
	void BindFramebuffer(u32 handle, FramebufferTarget target);
	b32 ValidateFramebuffer(FramebufferTarget type);

	enum FramebufferAttachment : u32
	{
		FB_ATTACHMENT_COLOR0,
		FB_ATTACHMENT_DEPTH
	};
	
	void FramebufferAttachTexture(FramebufferTarget target,
								  FramebufferAttachment att,
								  u32 textureHandle);

	
	void FramebufferAttachTextureMultisample(FramebufferTarget target,
											 FramebufferAttachment att,
											 u32 textureHandle);
}
