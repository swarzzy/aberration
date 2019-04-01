#include "GraphicsAPI.h"
#include "OpenGL/OpenGL.h"
#include "utils/Log.h"

namespace AB::API {
	struct OpenglTextureFormat {
		GLint internalFormat;
		GLint format;
	};
	static OpenglTextureFormat OpenglResolveTexFormat(PixelFormat f) {
		GLint intFormat;
		GLint format;
		switch (f) {
		case PixelFormat::RED: {
			intFormat = GL_R8;
			format = GL_RED;
		} break;
		case PixelFormat::RGB: {
			intFormat = GL_RGB8;
			format = GL_RGB;
		} break;
		case PixelFormat::RGBA: {
			intFormat = GL_RGBA8;
			format = GL_RGBA;
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
		case TextureFilter::Linear: { filter = GL_LINEAR; } break;
		case TextureFilter::Nearest: { filter = GL_NEAREST; } break;
		default: {
			AB_CORE_WARN("Unrecognized texture filter mode. Setting default value: Linear");
			filter = GL_LINEAR;
		} break;
		};

		switch (p.wrapMode) {
		case TextureWrapMode::Repeat:  { wrap = GL_REPEAT; } break;
		case TextureWrapMode::MirroredRepeat: { wrap = GL_MIRRORED_REPEAT; } break;
		case TextureWrapMode::ClampToEdge: { wrap = GL_CLAMP_TO_EDGE; } break;
		default: {
			AB_CORE_WARN("Unrecognized texture wrap mode> Setting default value: Repeat");
				wrap = GL_REPEAT;	
		} break;
		};

		return {filter, wrap};
	}
	
	uint32 CreateCubemap(TextureParameters params,
						 Image px, Image nx,
						 Image py, Image ny,
						 Image pz, Image nz) {
		uint32 resultHandle;
		GLuint texHandle;
		GLCall(glGenTextures(1, &texHandle));
		if (texHandle) {
			GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, texHandle));

			OpenglTextureFormat f = OpenglResolveTexFormat(px.format);	
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,
								0,
								f.internalFormat,
								px.width,
								px.height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								px.bitmap
								));
			
			f = OpenglResolveTexFormat(nx.format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
								0,
								f.internalFormat,
								nx.width,
								nx.height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								nx.bitmap
								));

			f = OpenglResolveTexFormat(py.format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
								0,
								f.internalFormat,
								py.width,
								py.height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								py.bitmap
								));

			f = OpenglResolveTexFormat(ny.format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
								0,
								f.internalFormat,
								ny.width,
								ny.height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								ny.bitmap
								));

			f = OpenglResolveTexFormat(pz.format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
								0,
								f.internalFormat,
								pz.width,
								pz.height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								pz.bitmap
								));

			f = OpenglResolveTexFormat(nz.format);
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
								0,
								f.internalFormat,
								nz.width,
								nz.height,
								0,
								f.format,
								GL_UNSIGNED_BYTE,
								nz.bitmap
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

			resultHandle = SafeCastIntI32(texHandle);
		} else {
			AB_CORE_ERROR("Failed to create cubemap texture: OpenGL API error");
		}
		return resultHandle;

	}
}
