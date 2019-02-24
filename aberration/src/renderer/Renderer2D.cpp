#include "Renderer2D.h"
#include "src/utils/Log.h"
#include <hypermath.h>
#include "src/platform/API/OpenGL/ABOpenGL.h"
#include "src/platform/Window.h"
#include "src/utils/ImageLoader.h"
#include "platform/Platform.h"
#include "utils/DebugTools.h"

namespace AB {
	const char* SPRITE_VERTEX_SOURCE = R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec4 aColor;
		layout (location = 2) in vec2 aUV;
		out vec4 v_Color;
		out vec2 v_UV;
		void main()
		{
			v_UV = aUV;
			v_Color = aColor;
		   gl_Position = vec4(aPos, 1.0, 1.0);
		}
	)";

	const char* SPRITE_FRAGMENT_SOURCE = R"(
		#version 330 core
		#extension GL_ARB_shader_subroutine : enable
		
		subroutine vec4 FetchPixelType();
		subroutine uniform FetchPixelType FetchPixel;
		
		out vec4 FragColor;
		in vec4 v_Color;
		in vec2 v_UV;
		uniform sampler2D sys_Texture;
		
		subroutine(FetchPixelType)
		vec4 FetchPixelTexture() {
			return texture(sys_Texture, v_UV);
		}

		subroutine(FetchPixelType)
		vec4 FetchPixelSolid() {
			return v_Color;
		}

		subroutine(FetchPixelType)
		vec4 FetchPixelGlyph() {
			vec4 texColor = texture(sys_Texture, v_UV);
			return vec4(v_Color.r, v_Color.g, v_Color.b, texColor.r * v_Color.a);
		}

		void main()
		{
			FragColor = FetchPixel();
		}
	)";

	static constexpr uint32 INVALID_TEXTURE_COLOR = 0xffff00ff;

	struct VertexData {
		float32 x;
		float32 y;
		color32 color;
		float32 u;
		float32 v;
	};

	enum class DrawableType : uint8 {
		Textured = 0,
		Glyph,
		SolidColor
	};

	struct BatchData {
		uint32 count;
		uint16 textureHandle;
		DrawableType type;
	};

	struct RectangleData {
		hpm::Vector2 position;
		hpm::Vector2 size;
		float32 angle;
		float32 anchor;
		uint32 color;
		uint16 regionTexHandle;
		DrawableType type;
	};

	struct UV {
		hpm::Vector2 min;
		hpm::Vector2 max;
	};

	struct TextureProperties {
		bool32 used;
		GLuint glHandle;
		uint32 refCount;
		PixelFormat format;
		UV uv;
		uint16 parent;
	};

	union SortKey {
		uint32 value;
		struct {
			uint16 texHandle;
			uint16 depth;
		};
	};

	struct SortEntry {
		SortKey key;
		uint32 renderQueueIndex;
	};

	struct Glyph {
		uint16 _reserved;
		uint16 regionHandle;
		float32 advance;
		float32 xBearing;
		float32 yBearing;
	};

	struct Font {
		static constexpr uint32 MAX_UNICODE_CHARACTER = 0x10ffff;
		static constexpr uint16 UNDEFINED_CODEPOINT = 0xffff;
		uint16 atlasHandle;
		uint32 atlasWidth;
		uint32 atlasHeight;
		float32 lineAdvance;
		float32 heightInPixels;
		uint32 numCodepoints;
		// NOTE: Needed only for kerning
		float32 scaleFactor;
		int16 kernTable[Renderer2D::FONT_MAX_CODEPOINTS * Renderer2D::FONT_MAX_CODEPOINTS];
		Glyph glyphs[Renderer2D::FONT_MAX_CODEPOINTS];
		// TODO: Allocate only memory we use
		// MAX_USED_CODEPOINT
		uint16 unicodeLookupTable[MAX_UNICODE_CHARACTER];
		uint16 GetGlyphIndex(uint32 unicodeCodepoint);
		float32 GetPairHorizontalAdvanceUnscaled(uint16 glyphIndex1, uint16 glyphIndex2);
	};

	struct Renderer2DProperties {
		uint32 drawCallCount;
		uint32 verticesDrawnCount;
		// TEMRORARY
		uint32 GLVBOHandle;
		uint32 GLIBOHandle;
		uint32 shaderHandle;
		GLuint subroutineGlyphIndex;
		GLuint subroutineSolidIndex;
		GLuint subroutineTextureIndex;
		GLuint uniformSamplerIndex;

		hpm::Vector2 viewSpaceDim;
		uint64 vertexCount;
		uint64 indexCount;
		uint16 texturesUsed;
		TextureProperties textures[Renderer2D::TEXTURE_STORAGE_CAPACITY];
		uint32 batchesUsed;
		uint16 drawQueueUsed;
		uint32 sortBufferUsage;
		BatchData batches[AB::Renderer2D::DRAW_QUEUE_CAPACITY];
		// TODO: Maybe write vertex data directly to the gpu address space using glMapBuffer
		// instead of having this vertex buffer
		VertexData vertexBuffer[AB::Renderer2D::DRAW_QUEUE_CAPACITY * 4];
		RectangleData drawQueue[AB::Renderer2D::DRAW_QUEUE_CAPACITY];
		SortEntry sortBufferA[AB::Renderer2D::DRAW_QUEUE_CAPACITY];
		SortEntry sortBufferB[AB::Renderer2D::DRAW_QUEUE_CAPACITY];
		// TEMPORARY: 
		// TODO: make font storage dynamically grown?
		uint16 fontsUsed;
		Font fonts[Renderer2D::FONT_STORAGE_SIZE];
	};

	static void _GLInit(Renderer2DProperties* properties);

	typedef bool32(MergeSortPred)(const SortKey* a, const SortKey* b);

	static void MergeSortV2(SortEntry* buffer, SortEntry* tempBuffer, uint32 beg, uint32 end, MergeSortPred* pred) {
		if (beg < end) {
			uint32 mid = beg + (end - beg) / 2;
			MergeSortV2(buffer, tempBuffer, beg, mid, pred);
			MergeSortV2(buffer, tempBuffer, mid + 1, end, pred);

			uint32 leftAt = beg;
			uint32 rightAt = mid + 1;
			uint32 tempAt = 0;
			while (leftAt <= mid && rightAt <= end) {
				AB_CORE_ASSERT(tempAt <= end, "Merge sort error. Buffer owerflow.");
				if (pred(&buffer[leftAt].key, &buffer[rightAt].key)) {
					tempBuffer[tempAt] = buffer[leftAt];
					tempAt++;
					leftAt++;
				}
				else {
					tempBuffer[tempAt] = buffer[rightAt];
					tempAt++;
					rightAt++;
				}
			}
			while (leftAt <= mid) {
				tempBuffer[tempAt] = buffer[leftAt];
				tempAt++;
				leftAt++;
			}
			while (rightAt <= end) {
				tempBuffer[tempAt] = buffer[rightAt];
				tempAt++;
				rightAt++;
			}

			memcpy(&buffer[beg], tempBuffer, sizeof(SortEntry) * (end - beg + 1));
		}
	}

	static SortEntry* MergeSortV3(SortEntry* bufferA, SortEntry* bufferB, uint32 beg, uint32 end, MergeSortPred* pred) {
		SortEntry* targetBuffer = bufferA;
		if (beg < end) {
			uint32 mid = beg + (end - beg) / 2;
			SortEntry* left = MergeSortV3(bufferA, bufferB, beg, mid, pred);
			SortEntry* right = MergeSortV3(bufferA, bufferB, mid + 1, end, pred);

			targetBuffer = left == bufferA ? bufferB : bufferA;

			uint32 leftAt = beg;
			uint32 rightAt = mid + 1;
			uint32 tempAt = beg;
			while (leftAt <= mid && rightAt <= end) {
				AB_CORE_ASSERT(tempAt <= end, "Merge sort error. Buffer overflow.");
				if (pred(&left[leftAt].key, &right[rightAt].key)) {
					targetBuffer[tempAt] = left[leftAt];
					tempAt++;
					leftAt++;
				}
				else {
					targetBuffer[tempAt] = right[rightAt];
					tempAt++;
					rightAt++;
				}
			}
			while (leftAt <= mid) {
				targetBuffer[tempAt] = left[leftAt];
				tempAt++;
				leftAt++;
			}
			while (rightAt <= end) {
				targetBuffer[tempAt] = right[rightAt];
				tempAt++;
				rightAt++;
			}
		}
		return targetBuffer;
	}

	// NOTE: Test if return pointer to UV from main texture buffer if faster
	static UV GetTextureRegionUV(Renderer2DProperties* properties, uint16 handle) {
		// TODO: is there any elegant way to initialize non POD struct with zeros?
		UV uv;
		memset(&uv, 0, sizeof(UV));
		if (handle > 0) {
			if (handle > 0) {
				uint16 index = handle - 1;
				uv = properties->textures[index].uv;
			}
		}
		return uv;
	}

	// TODO: Maybe store gl handles in regions too
	static uint32 GetTextureRegionAPIHandle(Renderer2DProperties* properties, uint16 regionHandle) {
		uint32 apiHandle = 0;
		if (regionHandle > 0) {
			uint16 index = regionHandle - 1;
			if (properties->textures[index].parent == 0) {
				apiHandle = properties->textures[index].glHandle;
			} else {
				apiHandle = properties->textures[properties->textures[index].parent - 1].glHandle;
			}
		}
		return apiHandle;
	}

	// NOTE: Returns handle! Not an actual index
	static uint16 GetTextureBaseHandle(Renderer2DProperties* properties, uint16 handle) {
		uint16 baseHandle = 0;
		if (handle > 0) {
			uint16 index = handle - 1;
			if (handle > 0) {
				if (properties->textures[index].parent == 0) {
					baseHandle = handle;
				}
				else {
					baseHandle = properties->textures[index].parent;
				}
			}
		}
		return baseHandle;
	}

	uint16 Renderer2D::LoadTexture(const char* filepath) {
		uint16 resultHandle = 0;

		bool32 hasFreeCell = false;
		uint16 freeIndex = 0;
		for (uint32 i = 0; i < TEXTURE_STORAGE_CAPACITY; i++) {
			if (!s_Properties->textures[i].used) {
				hasFreeCell = true;
				freeIndex = i;
				break;
			}
		}

		if (hasFreeCell) {
			Image image = LoadBMP(filepath);
			if (image.bitmap) {
				GLuint texHandle;
				uint32 format = GL_RED;
				uint32 inFormat = GL_RED;
				switch (image.format) {
				case PixelFormat::RGB: {
					format = GL_RGB;
					inFormat = GL_RGB8;
				} break;
				case PixelFormat::RGBA: {
					format = GL_RGBA;
					inFormat = GL_RGBA8;
				} break;
				default: {
					// TODO: FIX THIS
					AB_CORE_ERROR("Wrong image format");
				} break;
				}
				GLCall(glGenTextures(1, &texHandle));
				GLCall(glBindTexture(GL_TEXTURE_2D, texHandle));

				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GLCall(glTexImage2D(GL_TEXTURE_2D, 0, inFormat, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.bitmap));

				GLCall(glBindTexture(GL_TEXTURE_2D, 0));

				s_Properties->textures[freeIndex].used = true;
				s_Properties->textures[freeIndex].refCount = 1;
				s_Properties->textures[freeIndex].glHandle = texHandle;
				s_Properties->textures[freeIndex].format = image.format;
				s_Properties->textures[freeIndex].uv.min = hpm::Vector2{ 0.0f, 0.0f };
				s_Properties->textures[freeIndex].uv.max = hpm::Vector2{ 1.0f, 1.0f };
				s_Properties->textures[freeIndex].parent = 0;

				// NOTE: Increasing handle by one in order to use 0 value as invalid handle;
				resultHandle = freeIndex + 1;

				DeleteBitmap(image.bitmap);
			}
			else {
				AB_CORE_ERROR("Failed to load texture. Cannot load image: %s", filepath);
			}
		}
		else {
			AB_CORE_ERROR("Failed to load texture. No more space for textures!. Storage capacity: %u16", TEXTURE_STORAGE_CAPACITY);
		}
		return resultHandle;
	}

	uint16 Renderer2D::LoadTextureFromBitmap(PixelFormat format, uint32 width, uint32 height, const byte* bitmap) {
		uint16 resultHandle = 0;

		if (bitmap) {
			bool32 hasFreeCell = false;
			uint16 freeIndex = 0;
			for (uint32 i = 0; i < TEXTURE_STORAGE_CAPACITY; i++) {
				if (!s_Properties->textures[i].used) {
					hasFreeCell = true;
					freeIndex = i;
					break;
				}
			}

			if (hasFreeCell) {
				GLuint texHandle;
				uint32 glGormat = GL_RED;
				uint32 glInternalFormat = GL_RED;
				switch (format) {
				case PixelFormat::RGB: {
					glGormat = GL_RGB;
					glInternalFormat = GL_RGB8;
				} break;
				case PixelFormat::RGBA: {
					glGormat = GL_RGBA;
					glInternalFormat = GL_RGBA8;
				} break;
				case PixelFormat::RED: {
					glGormat = GL_RED;
					glInternalFormat = GL_R8;
				} break;
				default: {
					// TODO: FIX THIS
					AB_CORE_ERROR("Wrong image format");
				} break;
				}

				GLCall(glGenTextures(1, &texHandle));
				GLCall(glBindTexture(GL_TEXTURE_2D, texHandle));

				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				GLCall(glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glGormat, GL_UNSIGNED_BYTE, bitmap));

				GLCall(glBindTexture(GL_TEXTURE_2D, 0));

				s_Properties->textures[freeIndex].used = true;
				s_Properties->textures[freeIndex].refCount = 1;
				s_Properties->textures[freeIndex].glHandle = texHandle;
				s_Properties->textures[freeIndex].format = format;
				s_Properties->textures[freeIndex].uv.min = hpm::Vector2{ 0.0f, 0.0f };
				s_Properties->textures[freeIndex].uv.max = hpm::Vector2{ 1.0f, 1.0f };
				s_Properties->textures[freeIndex].parent = 0;

				// NOTE: Increasing handle by one in order to use 0 value as invalid handle;
				resultHandle = freeIndex + 1;
			}
			else {
				AB_CORE_ERROR("Failed to load texture. No more space for textures!. Storage capacity: %u16", TEXTURE_STORAGE_CAPACITY);
			}
		}
		return resultHandle;
	}

	void Renderer2D::FreeTexture(uint16 handle) {
		if (handle > 0) {
			uint16 index = handle - 1;
			s_Properties->textures[index].refCount--;
			if (s_Properties->textures[index].refCount == 0) {
				GLCall(glDeleteTextures(1, &s_Properties->textures[handle].glHandle));
				s_Properties->textures[index].used = false;
			}
		}
	}

	uint16 Renderer2D::TextureCreateRegion(uint16 handle, hpm::Vector2 min, hpm::Vector2 max) {

		uint16 resultHandle = 0;
		if (handle > 0) {

			bool32 hasFreeCell = false;
			uint16 freeIndex = 0;
			for (uint32 i = 0; i < TEXTURE_STORAGE_CAPACITY; i++) {
				if (!s_Properties->textures[i].used) {
					hasFreeCell = true;
					freeIndex = i;
					break;
				}
			}

			if (hasFreeCell) {
				s_Properties->textures[freeIndex].used = true;
				s_Properties->textures[freeIndex].glHandle = 0;
				s_Properties->textures[freeIndex].refCount = 1;
				s_Properties->textures[freeIndex].parent = handle;
				s_Properties->textures[freeIndex].uv.min = min;
				s_Properties->textures[freeIndex].uv.max = max;
				s_Properties->textures[handle - 1].refCount++;
				// TODO: method for delete region and decrease parent ref count

				resultHandle = freeIndex + 1;
			}
		}
		return resultHandle;
	}

	PixelFormat Renderer2D::GetTextureFormat(uint16 handle) {
		if (handle > 0) {
			return s_Properties->textures[handle - 1].format;
		}
		else {
			// TODO: Distinct enum for textures and return some error value
			return  PixelFormat::RED;
		}
	}

	void Renderer2D::Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY) {
		if (!s_Properties) {
			s_Properties = (Renderer2DProperties*)std::malloc(sizeof(Renderer2DProperties));
			memset(s_Properties, 0, sizeof(Renderer2DProperties));
		} else {
			AB_CORE_WARN("2D renderer already initialized.");
		}
		_GLInit(s_Properties);
		s_Properties->viewSpaceDim = hpm::Vector2{ (float32)drawableSpaceX, (float32)drawableSpaceY };
		// TODO: TEMPORARY
		LoadFont("../../../assets/SourceCodePro.abf");
	}

	static void _GLInit(Renderer2DProperties* properties) {
		uint32 winWidth;
		uint32 winHeight;
		Window::GetSize(&winWidth, &winHeight);
		GLCall(glViewport(0, 0, winWidth, winHeight));

		GLCall(glGenBuffers(1, &properties->GLVBOHandle));

		uint16* indices = (uint16*)std::malloc(Renderer2D::INDEX_BUFFER_SIZE * sizeof(uint16));
		uint16 k = 0;
		// TODO: IMPORTATNT: This alorithm might work incorrect on different sizes of index buffer
		for (uint32 i = 0; i < Renderer2D::INDEX_BUFFER_SIZE - 9; i += 6) {
			indices[i] = k;
			indices[i + 1] = k + 1;
			indices[i + 2] = k + 3;
			indices[i + 3] = k + 1;
			indices[i + 4] = k + 2;
			indices[i + 5] = k + 3;
			k += 4;
		}
		indices[Renderer2D::INDEX_BUFFER_SIZE - 1] = k;
		indices[Renderer2D::INDEX_BUFFER_SIZE - 2] = k + 1;

		GLCall(glGenBuffers(1, &properties->GLIBOHandle));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, properties->GLIBOHandle));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * Renderer2D::INDEX_BUFFER_SIZE, indices, GL_STATIC_DRAW));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		std::free(indices);

		int32 spriteVertexShader;
		GLCall(spriteVertexShader = glCreateShader(GL_VERTEX_SHADER));
		GLCall(glShaderSource(spriteVertexShader, 1, &SPRITE_VERTEX_SOURCE, 0));
		GLCall(glCompileShader(spriteVertexShader));

		int32 spritefragmentShader;
		GLCall(spritefragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
		GLCall(glShaderSource(spritefragmentShader, 1, &SPRITE_FRAGMENT_SOURCE, 0));
		GLCall(glCompileShader(spritefragmentShader));

		int32 result = 0;
		GLCall(glGetShaderiv(spriteVertexShader, GL_COMPILE_STATUS, &result));
		if (!result) {
			int32 logLen;
			GLCall(glGetShaderiv(spriteVertexShader, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			GLCall(glGetShaderInfoLog(spriteVertexShader, logLen, NULL, message));
			AB_CORE_FATAL("Shader compilation error:\n%s", message);
		};

		GLCall(glGetShaderiv(spritefragmentShader, GL_COMPILE_STATUS, &result));
		if (!result) {
			int32 logLen;
			GLCall(glGetShaderiv(spritefragmentShader, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			GLCall(glGetShaderInfoLog(spritefragmentShader, logLen, NULL, message));
			AB_CORE_FATAL("Shader compilation error:\n%s", message);
		};

		GLCall(properties->shaderHandle = glCreateProgram());
		GLCall(glAttachShader(properties->shaderHandle, spriteVertexShader));
		GLCall(glAttachShader(properties->shaderHandle, spritefragmentShader));
		GLCall(glLinkProgram(properties->shaderHandle));
		GLCall(glGetProgramiv(properties->shaderHandle, GL_LINK_STATUS, &result));
		if (!result) {
			int32 logLen;
			GLCall(glGetProgramiv(properties->shaderHandle, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			GLCall(glGetProgramInfoLog(properties->shaderHandle, logLen, 0, message));
			AB_CORE_FATAL("Shader compilation error:\n%s", message);
		}

		GLCall(glDeleteShader(spriteVertexShader));
		GLCall(glDeleteShader(spritefragmentShader));

		GLCall(glUseProgram(properties->shaderHandle));
		GLCall(properties->subroutineTextureIndex = glGetSubroutineIndexARB(properties->shaderHandle, GL_FRAGMENT_SHADER, "FetchPixelTexture"));
		GLCall(properties->subroutineGlyphIndex = glGetSubroutineIndexARB(properties->shaderHandle, GL_FRAGMENT_SHADER, "FetchPixelGlyph"));
		GLCall(properties->subroutineSolidIndex = glGetSubroutineIndexARB(properties->shaderHandle, GL_FRAGMENT_SHADER, "FetchPixelSolid"));

		GLCall(properties->uniformSamplerIndex = glGetUniformLocation(properties->shaderHandle, "sys_Texture"));
	}


	void RenderGroupResizeCallback(uint32 width, uint32 height) {
		if (Renderer2D::s_Properties) {

			GLCall(glViewport(0, 0, width, height));
		}
	}

	void Renderer2D::Destroy() {
		GLCall(glDeleteBuffers(1, &s_Properties->GLVBOHandle));
		GLCall(glDeleteBuffers(1, &s_Properties->GLIBOHandle));
		GLCall(glDeleteProgram(s_Properties->shaderHandle));
		std::free(s_Properties);
		s_Properties = nullptr;
	}

	uint32 Renderer2D::GetDrawCallCount() {
		return s_Properties->drawCallCount;
	}

	void Renderer2D::FillRectangleColor(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color) {
		if (s_Properties->sortBufferUsage <= DRAW_QUEUE_CAPACITY) {
			// Has alpha < 1.0f
			SortKey key = {};
			key.depth = depth;
			key.texHandle = 0;
			s_Properties->drawQueue[s_Properties->drawQueueUsed] = { position, size, angle, anchor, color, 0, DrawableType::SolidColor };
			s_Properties->sortBufferA[s_Properties->sortBufferUsage] = { key, s_Properties->drawQueueUsed };
			s_Properties->sortBufferUsage++;
			s_Properties->drawQueueUsed++;
		}
		else {
			AB_CORE_WARN("Failed to submit rectangle. Draw queue is full");
		}
	}

	void Renderer2D::FillRectangleTexture(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle) {
		if (s_Properties->sortBufferUsage <= DRAW_QUEUE_CAPACITY) {
			uint16 baseTexHandle = GetTextureBaseHandle(s_Properties, textureHandle);
			// NOTE: If texture handle is invalid the treat this rect as solid colored
			// See NOTE in GenVertexAndBatchBuffers()
			if (textureHandle > 0 && baseTexHandle > 0) {
				SortKey key = {};
				key.depth = depth;
				key.texHandle = baseTexHandle;
				s_Properties->drawQueue[s_Properties->drawQueueUsed] = { position, size,  angle, anchor, 0, textureHandle, DrawableType::Textured };
				s_Properties->sortBufferA[s_Properties->sortBufferUsage] = { key, s_Properties->drawQueueUsed };
				s_Properties->sortBufferUsage++;
				s_Properties->drawQueueUsed++;
			}
			else {
				FillRectangleColor(position, depth, angle, anchor, size, INVALID_TEXTURE_COLOR);
			}
		}
		else {
			AB_CORE_WARN("Failed to submit rectangle. Draw queue is full");
		}
	}

	bool32 Renderer2D::DrawRectangleColorUI(hpm::Vector2 min, hpm::Vector2 max, uint16 depth, float32 angle, float32 anchor, color32 color) {
		// TODO: THIS is temporary
		uint32 w;
		uint32 h;
		AB::Window::GetSize(&w, &h);
		uint32 xMouse = 0;
		uint32 yMouse = 0;
		Window::GetMousePosition(&xMouse, &yMouse);
		float32 xMouseInCanvasSpace = (float32)xMouse * (float32)s_Properties->viewSpaceDim.x / w;
		float32 yMouseInCanvasSpace = (float32)yMouse * (float32)s_Properties->viewSpaceDim.y / h;
		bool32 intersects = false;
		if (xMouseInCanvasSpace > min.x && xMouseInCanvasSpace < min.x + max.x &&
			yMouseInCanvasSpace > min.y && yMouseInCanvasSpace < min.y + max.y) 
		{
			intersects = true;
		}
		FillRectangleColor(min, depth, angle, anchor, max, color);
		return intersects;
	}

	inline static bool32 SortPred(const SortKey* a, const SortKey* b) {
		return a->value < b->value;
	}

	SortEntry* SortQueue(Renderer2DProperties* properties) {
		// NOTE: end is pointing to a last element
		// TODO: Copying the buffer because merge sort desn't. So if merge sort returns buffer B
		// then there is no left-side part of the queue.
		// Maybe just copy while sorting (MergeSortV2) more efficient?
		SortEntry* sortedBuffer = properties->sortBufferA;
		memcpy(properties->sortBufferB, properties->sortBufferA, properties->sortBufferUsage * sizeof(SortEntry));
		if (properties->sortBufferUsage > 2) {
			sortedBuffer = MergeSortV3(properties->sortBufferA, properties->sortBufferB, 0, properties->sortBufferUsage - 1, SortPred);
		}
		return  sortedBuffer;
	}

	static void GenVertexData(Renderer2DProperties* properties, RectangleData* rect, int16 depth) {
		// TODO: is there any elegant way to initialize non POD struct with zeros?
		UV uv;
		memset(&uv, 0, sizeof(UV));
		// NOTE: It's just a bit faster to not call this function if handle is 0
		if (rect->regionTexHandle != 0) {
			// TODO: There are two different handles now. Base handle in a sort entry and region handle in the draw queue
			uv = GetTextureRegionUV(properties, rect->regionTexHandle);
		}

		float32 sin = hpm::Sin(hpm::ToRadians(rect->angle));
		float32 cos = hpm::Cos(hpm::ToRadians(rect->angle));
		float32 invHalfW = (2 / properties->viewSpaceDim.x);
		float32 invHalfH = (2 / properties->viewSpaceDim.y);

		// rotation			// scale // translation	// ortho projection				   
		float32 originLBX = (((0 + rect->anchor) * cos - (0 + rect->anchor) * sin) * rect->size.x + rect->position.x) * invHalfW - 1;
		float32 originLBY = (((0 + rect->anchor) * sin + (0 + rect->anchor) * cos) * rect->size.y + rect->position.y) * invHalfH - 1;
		float32 originRBX = (((1 + rect->anchor) * cos - (0 + rect->anchor) * sin) * rect->size.x + rect->position.x) * invHalfW - 1;
		float32 originRBY = (((1 + rect->anchor) * sin + (0 + rect->anchor) * cos) * rect->size.y + rect->position.y) * invHalfH - 1;
		float32 originRTX = (((1 + rect->anchor) * cos - (1 + rect->anchor) * sin) * rect->size.x + rect->position.x) * invHalfW - 1;
		float32 originRTY = (((1 + rect->anchor) * sin + (1 + rect->anchor) * cos) * rect->size.y + rect->position.y) * invHalfH - 1;
		float32 originLTX = (((0 + rect->anchor) * cos - (1 + rect->anchor) * sin) * rect->size.x + rect->position.x) * invHalfW - 1;
		float32 originLTY = (((0 + rect->anchor) * sin + (1 + rect->anchor) * cos) * rect->size.y + rect->position.y) * invHalfH - 1;


		properties->vertexBuffer[properties->vertexCount].x = originLBX;
		properties->vertexBuffer[properties->vertexCount].y = originLBY;
		properties->vertexBuffer[properties->vertexCount].color = rect->color;
		properties->vertexBuffer[properties->vertexCount].u = uv.min.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.min.y;
		properties->vertexCount++;
		//right bottom
		properties->vertexBuffer[properties->vertexCount].x = originRBX;
		properties->vertexBuffer[properties->vertexCount].y = originRBY;
		properties->vertexBuffer[properties->vertexCount].color = rect->color;
		properties->vertexBuffer[properties->vertexCount].u = uv.max.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.min.y;
		properties->vertexCount++;
		//right top
		properties->vertexBuffer[properties->vertexCount].x = originRTX;
		properties->vertexBuffer[properties->vertexCount].y = originRTY;
		properties->vertexBuffer[properties->vertexCount].color = rect->color;
		properties->vertexBuffer[properties->vertexCount].u = uv.max.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.max.y;
		properties->vertexCount++;
		// left top
		properties->vertexBuffer[properties->vertexCount].x = originLTX;
		properties->vertexBuffer[properties->vertexCount].y = originLTY;
		properties->vertexBuffer[properties->vertexCount].color = rect->color;
		properties->vertexBuffer[properties->vertexCount].u = uv.min.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.max.y;
		properties->vertexCount++;

		properties->indexCount += 6;
	}

	void GenVertexAndBatchBuffers(Renderer2DProperties* properties, SortEntry* sortedBuffer) {
		// NOTE: This batching system doesn`t checking for renderable type.
		// For example if there are no difference between solid color renderables 
		// and renderables with invalid texture handle (both have zero handle).
		// So just ckecking in FillRectangleTexture for valid texture. If texture invalid then submit rect as SolidColor
		uint32 batchCount = 0;
		for (uint64 i = 0; i < properties->sortBufferUsage; i++) {
			batchCount++;
			RectangleData* rect = &properties->drawQueue[sortedBuffer[i].renderQueueIndex];

			if (i + 1 == properties->sortBufferUsage ||                               // NOTE: Abort batch if it's the last sprite in the queue
				sortedBuffer[i].key.texHandle != sortedBuffer[i + 1].key.texHandle)  // Or if next sprite has different tex handle
			{
				// NOTE: This is base handle
				properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;
				properties->batches[properties->batchesUsed].type = rect->type;
				properties->batches[properties->batchesUsed].count = batchCount;

				properties->batchesUsed++;
				batchCount = 0;
			}
			GenVertexData(properties, rect, sortedBuffer[i].key.depth);
		}
	}

	static void ResetRenderState(Renderer2DProperties* properties) {
		properties->batchesUsed = 0;
		properties->vertexCount = 0;
		properties->indexCount = 0;
		properties->sortBufferUsage = 0;
		properties->drawQueueUsed = 0;
	}

	void Renderer2D::Flush() {
		SortEntry* sortedBuffer = SortQueue(s_Properties);
		GenVertexAndBatchBuffers(s_Properties, sortedBuffer);
		GLCall(glDisable(GL_DEPTH_TEST));
		GLCall(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
		// TODO: Requires GL_LESS Depth test with clear to 0.0 and range 0.0 - 1.0
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, s_Properties->GLVBOHandle));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * s_Properties->vertexCount, (void*)s_Properties->vertexBuffer, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Properties->GLIBOHandle));

		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));
		GLCall(glEnableVertexAttribArray(1));
		GLCall(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), (void*)(sizeof(float32) * 2)));
		GLCall(glEnableVertexAttribArray(2));
		GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float32) * 2 + sizeof(byte) * 4)));

		// Always using 0 slot
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glUniform1i(s_Properties->uniformSamplerIndex, 0));

		s_Properties->drawCallCount = 0;
		uint64 verticesDrawn = 0;
		for (uint64 i = 0; i < s_Properties->batchesUsed; i++) {
			BatchData* batch = &s_Properties->batches[i];
			if (batch->type == DrawableType::Textured) {
				GLCall(glUniformSubroutinesuivARB(GL_FRAGMENT_SHADER, 1, &s_Properties->subroutineTextureIndex));
				if (batch->textureHandle > 0) {
					GLCall(glBindTexture(GL_TEXTURE_2D, GetTextureRegionAPIHandle(s_Properties, batch->textureHandle)));
				}
			}
			else if (batch->type == DrawableType::Glyph) {
				GLCall(glUniformSubroutinesuivARB(GL_FRAGMENT_SHADER, 1, &s_Properties->subroutineGlyphIndex));
				GLCall(glBindTexture(GL_TEXTURE_2D, GetTextureRegionAPIHandle(s_Properties, batch->textureHandle)));
			}
			else if (batch->type == DrawableType::SolidColor) {
				GLCall(glUniformSubroutinesuivARB(GL_FRAGMENT_SHADER, 1, &s_Properties->subroutineSolidIndex));
			}
			GLCall(glDrawElements(GL_TRIANGLES, 6 * s_Properties->batches[i].count, GL_UNSIGNED_SHORT, (void*)verticesDrawn));
			s_Properties->drawCallCount++;
			verticesDrawn += 6 * s_Properties->batches[i].count * sizeof(uint16);

		}

		s_Properties->verticesDrawnCount = (uint32)verticesDrawn;
		ResetRenderState(s_Properties);
	}


#define AB_FONT_BITMAP_FORMAT_KEY (uint16)0x1234

#pragma pack(push, 1)
	struct ABFontBitmapHeader {
		uint16 format;
		float32 heightInPixels;
		uint32 numCodepoints;
		uint32 bitmapBeginOffset;
		uint32 kernTableOffset;
		float32 lineAdvance;
		float32 scaleFactor;
		uint16 bitmapWidth;
		uint16 bitmapHeight;
	};

	struct PackedGlyphData {
		uint32 unicodeCodepoint;
		uint16 minX;
		uint16 minY;
		uint16 maxX;
		uint16 maxY;
		float32 xBearing;
		float32 yBearing;
		float32 advance;
	};
#pragma pack(pop)

	uint16 Renderer2D::LoadFont(const char* filepath) {
		uint16 resultHandle = 0;
		if (s_Properties->fontsUsed < FONT_STORAGE_SIZE) {
			uint16 storageIndex = s_Properties->fontsUsed;
			uint32 bytes = 0;
			// TODO: read only header at the beginning
			byte* fileData = (byte*)AB::DebugReadFile(filepath, &bytes);
			if (fileData && bytes) {
				ABFontBitmapHeader* header = (ABFontBitmapHeader*)fileData;

				AB_CORE_ASSERT(header->numCodepoints <= FONT_MAX_CODEPOINTS, "Too many glyphs in font!");

				if (header->format == AB_FONT_BITMAP_FORMAT_KEY) {
										
					byte* bitmap = fileData + header->bitmapBeginOffset;
					uint64 bitmapSize = header->bitmapWidth * header->bitmapHeight;

					uint16 handle = LoadTextureFromBitmap(PixelFormat::RED, header->bitmapWidth, header->bitmapHeight, bitmap);

					if (handle) {

						// NOTE: Unpacking kerning table
						uint32 kernTabSize = header->numCodepoints * header->numCodepoints;
						int16* kernTabFileAt = (int16*)(fileData + header->kernTableOffset);
						for (uint32 i = 0; i < kernTabSize; i++) {
							s_Properties->fonts[storageIndex].kernTable[i] = kernTabFileAt[i];
						}
						memset(s_Properties->fonts[storageIndex].unicodeLookupTable, Font::UNDEFINED_CODEPOINT, Font::MAX_UNICODE_CHARACTER);
						PackedGlyphData* glyphs = (PackedGlyphData*)(header + 1);

						for (uint32 i = 0; i < header->numCodepoints; i++) {
							// NOTE: Unpacking and converting positions ion the bitmap to normalized texture UVs.
							// This process can be moved to font processor but it will increase 
							// size of font bitmap file.
							float32 minX = (float32)(glyphs[i].minX) / header->bitmapWidth;
							float32 maxY = (float32)(glyphs[i].minY) / header->bitmapHeight;
							float32 maxX = (float32)(glyphs[i].maxX) / header->bitmapWidth;
							float32 minY = (float32)(glyphs[i].maxY) / header->bitmapHeight;

							s_Properties->fonts[storageIndex].glyphs[i].advance = glyphs[i].advance;
							s_Properties->fonts[storageIndex].glyphs[i].xBearing = glyphs[i].xBearing;
							s_Properties->fonts[storageIndex].glyphs[i].yBearing = glyphs[i].yBearing;
							s_Properties->fonts[storageIndex].atlasWidth = header->bitmapWidth;
							s_Properties->fonts[storageIndex].atlasHeight = header->bitmapHeight;
							s_Properties->fonts[storageIndex].heightInPixels = header->heightInPixels;
							s_Properties->fonts[storageIndex].scaleFactor = header->scaleFactor;
							s_Properties->fonts[storageIndex].lineAdvance = header->lineAdvance;
							s_Properties->fonts[storageIndex].numCodepoints = header->numCodepoints;
							s_Properties->fonts[storageIndex].atlasHandle = handle;


							uint32 unicodeCodepoint = glyphs[i].unicodeCodepoint;
							// NOTE: Store glyph index + 1 in order to threat 0 as empty character
							s_Properties->fonts[storageIndex].unicodeLookupTable[unicodeCodepoint] = i;

							uint16 regionHandle = TextureCreateRegion(handle, hpm::Vector2{ minX, minY }, hpm::Vector2{ maxX, maxY });
							if (regionHandle) {
								// All success!
								resultHandle = storageIndex + 1;
								s_Properties->fonts[storageIndex].glyphs[i].regionHandle = regionHandle;
							}
							else {
								// failed to create region
								AB_CORE_ERROR("Missing character \"%c\" when creating font atlas.", (char)i);
								s_Properties->fonts[storageIndex].glyphs[i].regionHandle = 0;
							}
						}
					}
					else {
						AB_CORE_ERROR("Failed to load font. Failed to create font atlas. File: %s", filepath);
					}
				}
				else {
					AB_CORE_ERROR("Failed to load font. Wrong file format. File: %s", filepath);
				}
				AB::DebugFreeFileMemory(fileData);
			}
			else {
				AB_CORE_ERROR("Failed to load font. Can not open file: %s", filepath);
			}
		}
		return resultHandle;
	}

	inline float32 Font::GetPairHorizontalAdvanceUnscaled(uint16 glyphIndex1, uint16 glyphIndex2) {
		float32 advance = 0.0f;
		// TODO: Check is character actually in here
		if (glyphIndex1 != Font::UNDEFINED_CODEPOINT && glyphIndex2 != Font::UNDEFINED_CODEPOINT) {
			Glyph* glyph = &glyphs[glyphIndex1];
			advance += glyph->advance;
			if (glyphIndex2 >= 0) {
				advance += kernTable[numCodepoints * glyphIndex1 + glyphIndex2] * scaleFactor;
			}
		} else if (glyphIndex1 != Font::UNDEFINED_CODEPOINT) {
			Glyph* glyph = &glyphs[glyphIndex1];

			advance += glyph->advance;
		}
		return advance;
	}

	inline uint16 Font::GetGlyphIndex(uint32 unicodeCodepoint) {
		return unicodeLookupTable[unicodeCodepoint];
	}

	template <typename CharType>
	static void _DebugDrawStringInternal(Renderer2DProperties* properties, hpm::Vector2 position, float32 fontHeight, color32 color, const CharType* string) {
		// TODO: This is all temporary
		// Here are gonna be direct submission to a drawQueue and sortBuffer
		if (string) {
			Font* font = &properties->fonts[Renderer2D::DEFAULT_FONT_HANDLE - 1];
			// TODO: check if font is not loaded
			// This is actually doesn't work for now
			if (font) {
				float32 scale = fontHeight / font->heightInPixels;
				bool32 stringBegin = true;

				// Checking first string
				float32 yFirstLineMaxAscent = 0.0f;

				for (uint32 at = 0; string[at] != '\n' && string[at] != '\0'; at++) {
					uint16 glyphIndex = font->GetGlyphIndex((uint32)string[at]);
					if (glyphIndex != Font::UNDEFINED_CODEPOINT) {
						Glyph* glyph = &font->glyphs[glyphIndex];
						UV uv = GetTextureRegionUV(properties, glyph->regionHandle);

						float32 glyphHeight = (uv.min.y - uv.max.y) * font->atlasHeight * scale;
						float32 descent = glyphHeight - glyph->yBearing * scale;
						float32 ascent = glyphHeight - descent;
						yFirstLineMaxAscent = yFirstLineMaxAscent > ascent ? ascent : yFirstLineMaxAscent;
					}
					else {
						yFirstLineMaxAscent = yFirstLineMaxAscent > fontHeight ? fontHeight : yFirstLineMaxAscent;
					}
				}

				float32 xAdvance = position.x;
				float32 yAdvance = position.y + yFirstLineMaxAscent;

				for (uint32 at = 0; string[at]; at++) {
					if (string[at] == '\n') {
						yAdvance -= font->lineAdvance * scale;
						xAdvance = position.x;
						stringBegin = true;
					}
					else {
						uint16 glyphIndex = font->GetGlyphIndex((uint32)string[at]);
						if (glyphIndex != Font::UNDEFINED_CODEPOINT) {
							Glyph* glyph = &font->glyphs[glyphIndex];
							if (string[at] != ' ') {
								UV uv = GetTextureRegionUV(properties, glyph->regionHandle);

								float32 width = (uv.max.x - uv.min.x) * font->atlasWidth * scale;
								float32 height = (uv.min.y - uv.max.y) * font->atlasHeight * scale;
								float32 xPosition = stringBegin ? xAdvance : xAdvance + glyph->xBearing * scale;
								float32 yPosition = yAdvance - glyph->yBearing * scale;


								hpm::Vector2 quadPos = { xPosition, yPosition };
								hpm::Vector2 quadSize = { width, height };
								if (properties->sortBufferUsage <= Renderer2D::DRAW_QUEUE_CAPACITY) {
									SortKey key = {};
									key.depth = 10;
									key.texHandle = GetTextureBaseHandle(properties, glyph->regionHandle);
									properties->drawQueue[properties->drawQueueUsed] = { quadPos, quadSize, 0, 0, color, glyph->regionHandle, DrawableType::Glyph };
									properties->sortBufferA[properties->sortBufferUsage] = { key, properties->drawQueueUsed };
									properties->sortBufferUsage++;
									properties->drawQueueUsed++;
								}
								else {
									AB_CORE_WARN("Failed to submit rectangle. Draw queue is full");
								}
							}
							stringBegin = false;

							uint16 nextGlyphIndex = Font::UNDEFINED_CODEPOINT;
							if (string[at + 1] != '\n' && string[at + 1] != '\0') {
								nextGlyphIndex = font->GetGlyphIndex((uint32)string[at + 1]);;
							}
							// NOTE: at + 1 is safe because on last iteration we are not going over the bounds of the array
							// We just accessing \0
							xAdvance += scale * font->GetPairHorizontalAdvanceUnscaled(glyphIndex, nextGlyphIndex);
						}
						else {
							float32 xPosition = stringBegin ? xAdvance : xAdvance + fontHeight / 2;
							Renderer2D::FillRectangleColor(
								hpm::Vector2{ xPosition, yAdvance },
								10, 0, 0,
								hpm::Vector2{ fontHeight - (fontHeight / 2), fontHeight },
								0xffffffff
							);
							xAdvance += fontHeight - (fontHeight / 2);
						}
					}
				}
			} // font not foud
		}
	}

	template <typename CharType>
	static hpm::Rectangle _GetStringBoundingRectInternal(Renderer2DProperties* properties, hpm::Vector2 position, float32 height, const CharType* string) {
		hpm::Rectangle rect;
		// TODO: make vectors POD
		memset(&rect, 0, sizeof(hpm::Rectangle));
		if (string) {
			Font* font = &properties->fonts[Renderer2D::DEFAULT_FONT_HANDLE - 1];
			// TODO: check if font is not loaded
			// This is actually doesn't work for now
			if (font) {
				float32 scale = height / font->heightInPixels;

				float32 xMaxAdvance = 0.0f;
				// Checking first string
				float32 firstLineMaxAscent = 0.0f;
				float32 maxLineDescent = 0.0f;

				uint32 firstStringAt = 0;
				while (string[firstStringAt] != '\n' && string[firstStringAt] != '\0') {
					uint16 glyphIndex = font->GetGlyphIndex((uint32)string[firstStringAt]);;
					if (glyphIndex != Font::UNDEFINED_CODEPOINT) {
						Glyph* glyph = &font->glyphs[glyphIndex];
						UV uv = GetTextureRegionUV(properties, glyph->regionHandle);

						float32 glyphHeight = (uv.min.y - uv.max.y) * font->atlasHeight * scale;
						float32 descent = glyphHeight - glyph->yBearing * scale;
						float32 ascent = glyphHeight - descent;
						firstLineMaxAscent = firstLineMaxAscent > ascent ? ascent : firstLineMaxAscent;
						maxLineDescent = maxLineDescent > descent ? descent : maxLineDescent;

						uint16 nextGlyphIndex = Font::UNDEFINED_CODEPOINT;
						if (string[firstStringAt + 1] != '\n' && string[firstStringAt + 1] != '\0') {
							nextGlyphIndex = font->GetGlyphIndex((uint32)string[firstStringAt + 1]);
						}
						xMaxAdvance += scale * font->GetPairHorizontalAdvanceUnscaled(glyphIndex, nextGlyphIndex);

						firstStringAt++;
					}
					else {
						firstLineMaxAscent = firstLineMaxAscent > height ? height : firstLineMaxAscent;
						firstStringAt++;
					}
				}

				if (string[firstStringAt] != '\0') {
					float32 xAdvance = 0.0f;
					float32 yAdvance = 0.0f + firstLineMaxAscent;
					maxLineDescent = 0.0f;

					for (uint32 at = firstStringAt + 1; string[at]; at++) {
						if (string[at] == '\n') {
							yAdvance -= font->lineAdvance * scale;
							xMaxAdvance = xMaxAdvance < xAdvance ? xAdvance : xMaxAdvance;
							maxLineDescent = 0.0f;
							xAdvance = 0.0f;
						}
						else {
							uint16 glyphIndex = font->GetGlyphIndex((uint32)string[at]);;
							if (glyphIndex != Font::UNDEFINED_CODEPOINT) {
								Glyph* glyph = &font->glyphs[glyphIndex];
								UV uv = GetTextureRegionUV(properties, glyph->regionHandle);
								float32 glyphHeight = (uv.min.y - uv.max.y) * font->atlasHeight * scale;
								float32 descent = glyphHeight - glyph->yBearing * scale;
								maxLineDescent = maxLineDescent > descent ? descent : maxLineDescent;

								uint16 nextGlyphIndex = Font::UNDEFINED_CODEPOINT;
								if (string[at + 1] != '\n' && string[at + 1] != '\0') {
									nextGlyphIndex = font->GetGlyphIndex((uint32)string[at + 1]);;
								}
								xAdvance += scale * font->GetPairHorizontalAdvanceUnscaled(glyphIndex, nextGlyphIndex);

								xMaxAdvance = xMaxAdvance < xAdvance ? xAdvance : xMaxAdvance;
							}
							else {
								xAdvance += height - (height / 2);
								xMaxAdvance = xMaxAdvance < xAdvance ? xAdvance : xMaxAdvance;
							}
						}
					}
					rect.min.x = position.x;
					rect.max.x = position.x + xMaxAdvance;
					rect.min.y = position.y + (yAdvance - (font->lineAdvance * scale) + maxLineDescent);
					rect.max.y = position.y;
				}
				else {
					float32 newLineAdvance = 0.0f;
					if (string[firstStringAt] == '\n') {
						newLineAdvance = font->lineAdvance * scale;
					}
					rect.min.x = position.x;
					rect.max.x = position.x + xMaxAdvance;
					rect.min.y = position.y + (firstLineMaxAscent + maxLineDescent - newLineAdvance);
					rect.max.y = position.y;
				}
			}
		}
		return rect;
	}

	void Renderer2D::DebugDrawString(hpm::Vector2 position, float32 height, color32 color, const char* string) {
		_DebugDrawStringInternal<char>(s_Properties, position, height, color, string);
	}

	void Renderer2D::DebugDrawString(hpm::Vector2 position, float32 height, color32 color, const wchar_t* string) {
		_DebugDrawStringInternal<wchar_t>(s_Properties, position, height, color, string);
	}

	hpm::Rectangle Renderer2D::GetStringBoundingRect(hpm::Vector2 position, float32 height, const char* string) {
		return _GetStringBoundingRectInternal<char>(s_Properties, position, height, string);
	}
	hpm::Rectangle Renderer2D::GetStringBoundingRect(hpm::Vector2 position, float32 height, const wchar_t* string) {
		return _GetStringBoundingRectInternal<wchar_t>(s_Properties, position, height, string);
	}

#if 0
	SortEntry* RadixSort(SortEntry* source, SortEntry* dest, uint32  count) {
		for (uint32 byteIndex = 0; byteIndex < 4; byteIndex++) {
			uint32 sortKeyOffsets[256] = {};

			for (uint32 i = 0; i < count; i++) {
				uint32 radixByte = (source[i].key.value >> byteIndex * 8) & 0x000000ff;
				sortKeyOffsets[radixByte]++; 
			}

			uint32 total = 0;
			for (uint32 i = 0; i < 256; i++) {
				uint32 keyCount = sortKeyOffsets[i];
				sortKeyOffsets[i] = total;
				total += keyCount;
			}

			for (uint32 i = 0; i < count; i++) {
				uint32 radixByte = (source[i].key.value >> byteIndex * 8) & 0x000000ff;
				dest[sortKeyOffsets[radixByte]] = source[i];
				sortKeyOffsets[radixByte]++;
			}

			SortEntry* tmp = dest;
			dest = source;
			source = tmp;
		}
		return source;
	}
#endif
}