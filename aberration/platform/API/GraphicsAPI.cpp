#include "GraphicsAPI.h"
#include "OpenGL/OpenGL.h"
#include "utils/Log.h"
#include "utils/ImageLoader.h"

namespace AB::API {

	TextureFormat GetTextureFormat(uint32 bitsPerPixel, ColorSpace cs) {
		// TODO: Support more trexture formats
		TextureFormat result = TEX_FORMAT_RED8;
		if (bitsPerPixel == 8) {
			result = TEX_FORMAT_RED8;
		} else if (bitsPerPixel == 24) {
			if (cs == TEX_COLOR_SPACE_LINEAR) {
				result = TEX_FORMAT_RGB8;				
			} else {
				result = TEX_FORMAT_SRGB8;				
			}
		} else if (bitsPerPixel == 32) {
			if (cs == TEX_COLOR_SPACE_LINEAR) {
				result = TEX_FORMAT_RGBA8;							
			} else {
				result = TEX_FORMAT_SRGB8_A8;							
			}
		} else if (bitsPerPixel > 8) {
			AB_CORE_WARN("Unrecognized texture format. Assuming RED.");
		} else {
			AB_CORE_FATAL("Invalid texture format.");
		}
		return result;
	}
	
	struct OpenglTextureFormat {
		GLint internalFormat;
		GLint format;
	};
	
	static OpenglTextureFormat OpenglResolveTexFormat(TextureFormat f) {
		GLint intFormat;
		GLint format;
		switch (f) {
		case TEX_FORMAT_RED8: {
			intFormat = GL_R8;
			format = GL_RED;
		} break;
		case TEX_FORMAT_RGB8: {
			intFormat = GL_RGB8;				
			format = GL_RGB;
		} break;
		case TEX_FORMAT_RGBA8: {
			intFormat = GL_RGBA8;				
			format = GL_RGBA;
		} break;
		case TEX_FORMAT_RED16: {
			intFormat = GL_R16;				
			format = GL_RED;
		} break;
		case TEX_FORMAT_RGB16: {
			intFormat = GL_RGB16;				
			format = GL_RGB;
		} break;
		case TEX_FORMAT_RGBA16: {
			intFormat = GL_RGBA16;				
			format = GL_RGBA;
		} break;
		case TEX_FORMAT_SRGB8: {
			intFormat = GL_SRGB8;;				
			format = GL_RGB;
		} break;
		case TEX_FORMAT_SRGB8_A8: {
			intFormat = GL_SRGB8_ALPHA8;				
			format = GL_RGB;
		} break;
		case TEX_FORMAT_RGB16F: {
			intFormat = GL_RGB16F;				
			format = GL_RGB;
		} break;
		case TEX_FORMAT_RGBA16F: {
			intFormat = GL_RGBA16F;				
			format = GL_RGBA;
		} break;
		case TEX_FORMAT_DEPTH32: {
			intFormat = GL_DEPTH_COMPONENT32;				
			format = GL_DEPTH_COMPONENT;
		} break;
		default: {
			AB_CORE_WARN("Unrecognized image format");
			intFormat = 0;
			format = 0;
		} break;
		};
		return {intFormat, format};
	}

	struct OpenglTextureParams {
		GLint filterMode;
		GLint wrapMode;
	};

	static OpenglTextureParams OpenglResolveTexParams(TextureParameters p) {
		GLint filter;
		GLint wrap;
		switch (p.filterMode) {
		case TEX_FILTER_LINEAR: { filter = GL_LINEAR; } break;
		case TEX_FILTER_NEAREST: { filter = GL_NEAREST; } break;
		default: {
			AB_CORE_WARN("Unrecognized texture filter mode. Setting default value: Linear");
			filter = GL_LINEAR;
		} break;
		};

		switch (p.wrapMode) {
		case TEX_WRAP_REPEAT:  { wrap = GL_REPEAT; } break;
		case TEX_WRAP_MIRR_REPEAT: { wrap = GL_MIRRORED_REPEAT; } break;
		case TEX_WRAP_CLAMP_TO_EDGE: { wrap = GL_CLAMP_TO_EDGE; } break;
		default: {
			AB_CORE_WARN("Unrecognized texture wrap mode> Setting default value: Repeat");
				wrap = GL_REPEAT;	
		} break;
		};

		return {filter, wrap};
	}
	
	uint32 CreateCubemap(TextureParameters params,
						 Image* px, Image* nx,
						 Image* py, Image* ny,
						 Image* pz, Image* nz) {
		uint32 resultHandle;
		GLuint texHandle;
		GLCall(glGenTextures(1, &texHandle));
		if (texHandle) {
			GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, texHandle));

			OpenglTextureFormat f = OpenglResolveTexFormat(px->format);	
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,
								0,
								f.internalFormat,
								px->width,
								px->height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								px->bitmap
								));
			
			f = OpenglResolveTexFormat(nx->format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
								0,
								f.internalFormat,
								nx->width,
								nx->height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								nx->bitmap
								));

			f = OpenglResolveTexFormat(py->format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
								0,
								f.internalFormat,
								py->width,
								py->height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								py->bitmap
								));

			f = OpenglResolveTexFormat(ny->format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
								0,
								f.internalFormat,
								ny->width,
								ny->height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								ny->bitmap
								));

			f = OpenglResolveTexFormat(pz->format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
								0,
								f.internalFormat,
								pz->width,
								pz->height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								pz->bitmap
								));

			f = OpenglResolveTexFormat(nz->format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
								0,
								f.internalFormat,
								nz->width,
								nz->height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								nz->bitmap
								));

			OpenglTextureParams p = OpenglResolveTexParams(params);
			GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
								   p.filterMode));
			GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
								   p.filterMode));
			GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
								   p.wrapMode));
			GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
								   p.wrapMode));
			GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
								   p.wrapMode));

			resultHandle = SafeCastUintU32(texHandle);
		} else {
			AB_CORE_ERROR("Failed to create cubemap texture: OpenGL API error");
		}
		return resultHandle;

	}
	
	AB_API uint32 CreateTexture(TextureParameters p, TextureFormat f,
								uint32 w, uint32 h, void* bitmap ) {
		uint32 resultHandle;

		auto[inFormat, format] = OpenglResolveTexFormat(f);

		if (format) {
			GLuint handle;
			GLCall(glGenTextures(1, &handle));
			if (handle) {
				GLCall(glBindTexture(GL_TEXTURE_2D, handle));
				
				auto[filter, wrap] = OpenglResolveTexParams(p);
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));

				// TODO: Bug? Using GL_UUSIGNED_BYTE with float formats
				GLCall(glTexImage2D(GL_TEXTURE_2D, 0, inFormat, w, h,
									0, format, GL_UNSIGNED_BYTE, bitmap));

				GLCall(glBindTexture(GL_TEXTURE_2D, 0));

				resultHandle = handle;
			} else {
				AB_CORE_ERROR("Failed to create texture: OpenGL API error.");
			}
		} else {
			AB_CORE_ERROR("Failed to create texture. Unrecognized format.");
		}
		
		return SafeCastUintU32(resultHandle);		
	}

	AB_API uint32 CreateTextureMultisample(TextureParameters p, TextureFormat f,
										   uint32 w, uint32 h, uint32 samples)
	{
		uint32 resultHandle;

		auto[inFormat, format] = OpenglResolveTexFormat(f);

		if (format) {
			GLuint handle;
			GLCall(glGenTextures(1, &handle));
			if (handle) {
				GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, handle));
				
				auto[filter, wrap] = OpenglResolveTexParams(p);
				GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, wrap));
				GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, wrap));
				GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, filter));
				GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, filter));
				
				GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,
											   inFormat, w, h, GL_TRUE));

				GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));

				resultHandle = handle;
			} else {
				AB_CORE_ERROR("Failed to create texture: OpenGL API error.");
			}
		} else {
			AB_CORE_ERROR("Failed to create texture. Unrecognized format.");
		}
		
		return SafeCastUintU32(resultHandle);		
	}

	uint32 CreateTexture(TextureParameters p, Image* img) {
		return CreateTexture(p, img->format, img->width, img->height, img->bitmap);	
	}

	uint32 CreateFramebuffer() {
		GLuint handle;
		GLCall(glGenFramebuffers(1, &handle));
		if (handle) {
			return SafeCastUintU32(handle);
		} else {
			AB_CORE_ERROR("Failed to create framebuffer. API error.");
			return 0;
		}
	}

	void ClearCurrentFramebuffer(uint32 flags) {
		GLbitfield glFlags = 0;
		if (flags & CLEAR_COLOR) {
			glFlags |= GL_COLOR_BUFFER_BIT;	
		}
		if (flags & CLEAR_DEPTH) {
			glFlags |= GL_DEPTH_BUFFER_BIT;
		}
		GLCall(glClear(glFlags));
	}

	inline static GLenum _OpenglResolveFramebufferTarget(FramebufferTarget type) {
		GLenum target;
		switch (type) {
		case FB_TARGET_DRAW: { target = GL_DRAW_FRAMEBUFFER; } break;
		case FB_TARGET_READ: { target = GL_READ_FRAMEBUFFER;} break;
		case FB_TARGET_READ_DRAW: {target = GL_FRAMEBUFFER;} break;
		default: { AB_CORE_FATAL("Wrong framebuffer usage hint"); } break;
		}
		return target;
	}
	
	void BindFramebuffer(uint32 handle, FramebufferTarget type) {
		auto target = _OpenglResolveFramebufferTarget(type);
		GLCall(glBindFramebuffer(target, handle));
	}

	bool32 ValidateFramebuffer(FramebufferTarget type) {
		auto target = _OpenglResolveFramebufferTarget(type);
		bool32 result = false;
		GLCall(result = glCheckFramebufferStatus(target) == GL_FRAMEBUFFER_COMPLETE);
		return result;	
	}

	inline static GLenum _OpenglResolveFramebufferAttachment(FramebufferAttachment a) {
		GLenum att;
		switch (a) {
		case FB_ATTACHMENT_COLOR0: { att = GL_COLOR_ATTACHMENT0;} break;
		case FB_ATTACHMENT_DEPTH: { att = GL_DEPTH_ATTACHMENT;} break;
		default: {AB_CORE_FATAL("Wrong framebuffer attachment"); } break;
		}
		return att;
	}
	
	void FramebufferAttachTexture(FramebufferTarget target,
								  FramebufferAttachment att,
								  uint32 textureHandle)
	{
		auto glAtt = _OpenglResolveFramebufferAttachment(att);
		auto glTarget = _OpenglResolveFramebufferTarget(target);
		GLCall(glFramebufferTexture2D(glTarget, glAtt, GL_TEXTURE_2D, textureHandle, 0));
	}

	AB_API void FramebufferAttachTextureMultisample(FramebufferTarget target,
													FramebufferAttachment att,
													uint32 textureHandle)
	{
		auto glAtt = _OpenglResolveFramebufferAttachment(att);
		auto glTarget = _OpenglResolveFramebufferTarget(target);
		GLCall(glFramebufferTexture2D(glTarget, glAtt, GL_TEXTURE_2D_MULTISAMPLE, textureHandle, 0));	
	}

}
