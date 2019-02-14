#include "Renderer2D.h"
#include "src/utils/Log.h"
#include <hypermath.h>
#include "src/platform/API/OpenGL/ABOpenGL.h"
#include "src/platform/Window.h"
#include "src/utils/ImageLoader.h"
#include "platform/Platform.h"


namespace AB {
	const char* VERTEX_SOURCE = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec4 aColor;
		layout (location = 2) in vec2 aUV;
		out vec4 v_Color;
		out vec2 v_UV;
		void main()
		{
			v_UV = aUV;
			v_Color = aColor;
		   gl_Position = vec4(aPos, 1.0);
		}
	)";

	const char* FRAGMENT_SOURCE = R"(
		#version 330 core
		out vec4 FragColor;
		in vec4 v_Color;
		in vec2 v_UV;
		uniform sampler2D tex;
		uniform float useTexture;
		void main()
		{
			if (useTexture > 0.4) {
				FragColor = texture(tex, v_UV);
			} else {
				FragColor = v_Color;
			}
		}
	)";

	static void _GLInit(Renderer2DProperties* properties);

	struct VertexData {
		float32 x;
		float32 y;
		float32 z;
		color32 color;
		float32 u;
		float32 v;
	};

	struct BatchData {
		uint32 count;
		uint32 textureHandle;
	};

	struct RectangleData {
		hpm::Vector2 position;
		hpm::Vector2 size;
		float32 angle;
		float32 anchor;
		uint32 color;
		uint16 regionTexHandle;
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
		float32 yOffset;
	};

	struct Font {
		uint16 atlasHandle;
		uint32 atlasWidth;
		uint32 atlasHeight;
		uint32 numChars;
		Glyph glyphs[Renderer2D::FONT_NUMBER_OF_CHARACTERS];
	};

	struct Renderer2DProperties {
		// TEMRORARY
		uint32 GLVBOHandle;
		uint32 GLIBOHandle;
		uint32 shaderHandle;
		hpm::Vector2 viewSpaceDim;
		uint64 vertexCount;
		uint64 indexCount;
		uint16 texturesUsed;
		TextureProperties textures[Renderer2D::TEXTURE_STORAGE_CAPACITY];
		uint32 batchesUsed;
		uint16 drawQueueUsed;
		uint32 sortBufferLeftUsage;
		uint32 sortBufferRightUsage;
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

	// NOTE: Test if return pointer to UV from main texture buffer if faster
	static UV GetTextureRegionUV(Renderer2DProperties* properties, uint16 handle) {
		// TODO: is there any elegant way to initialize non POD struct with zeros?
		UV uv;
		memset(&uv, 0, sizeof(UV));
		if (handle > 0) {
			uint16 index = handle - 1;
			uv = properties->textures[index].uv;
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
		uint16 index = handle - 1;
		uint16 baseHandle = 0;
		if (handle > 0) {
			if (properties->textures[index].parent == 0) {
				baseHandle = handle;
			}
			else {
				baseHandle = properties->textures[index].parent;
			}
		}
		return baseHandle;
	}

	void Renderer2D::Initialize(uint32 drawableSpaceX, uint32 drawableSpaceY) {
		if (!s_Properties) {
			s_Properties = (Renderer2DProperties*)std::malloc(sizeof(Renderer2DProperties));
			memset(s_Properties, 0, sizeof(Renderer2DProperties));
			s_Properties->sortBufferRightUsage = DRAW_QUEUE_CAPACITY - 1;
		} else {
			AB_CORE_WARN("2D renderer already initialized.");
		}
		_GLInit(s_Properties);
		s_Properties->viewSpaceDim = hpm::Vector2((float32)drawableSpaceX, (float32)drawableSpaceY);
		// TODO: TEMPORARY
#if defined(AB_PLATFORM_WINDOWS)
		//LoadFont("a:\\dev\\aberration\\build\\bin\\Debug-windows\\arial.abf");
		LoadFont("arial.abf");

#else 
		LoadFont("arial.abf");
#endif
	}

	void Renderer2D::Destroy() {
		AB_GLCALL(glDeleteBuffers(1, &s_Properties->GLVBOHandle));
		AB_GLCALL(glDeleteBuffers(1, &s_Properties->GLIBOHandle));
		AB_GLCALL(glDeleteProgram(s_Properties->shaderHandle));
		std::free(s_Properties);
		s_Properties = nullptr;
	}

#define AB_FONT_BITMAP_FORMAT_KEY (uint16)0x1

#pragma pack(push, 1)
	struct ABFontBitmapHeader {
		uint16 format;
		uint32 numChars;
		uint32 bitmapBeginOffset;
		uint16 bitmapWidth;
		uint16 bitmapHeight;
		//float32 fontAscent;
		//float32 fontDescent;
	};

	struct PackedGlyphData {
		uint16 minX;
		uint16 minY;
		uint16 maxX;
		uint16 maxY;
		float32 yOffset;
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

				AB_CORE_ASSERT(header->numChars < FONT_NUMBER_OF_CHARACTERS, "Too many glyphs in font!");

				if (header->format == AB_FONT_BITMAP_FORMAT_KEY) {

					byte* bitmap = fileData + header->bitmapBeginOffset;
					uint64 bitmapSize = header->bitmapWidth * header->bitmapHeight;
					// TODO: Allocation!!!
					byte* bitmapRGBA = (byte*)std::malloc(bitmapSize * sizeof(byte) * 4);

					for (uint64 i = 0; i < bitmapSize; i++) {
						uint32* g = (uint32*)bitmapRGBA;
						g[i] = bitmap[i] << 24 | bitmap[i] << 16 | bitmap[i] << 8 | bitmap[i];
					}

					uint16 handle = LoadTextureFromBitmap(PixelFormat::RGBA, header->bitmapWidth, header->bitmapHeight, bitmapRGBA);

					if (handle) {

						PackedGlyphData* glyphs = (PackedGlyphData*)(header + 1);

						for (uint32 i = 0; i < header->numChars; i++) {
							// NOTE: Unpacking and converting positions ion the bitmap to normalized texture UVs.
							// This process can be moved to font processor but it will increase 
							// size of font bitmap file.
							float32 minX = (float32)(glyphs[i].minX) / header->bitmapWidth;
							float32 maxY = (float32)(glyphs[i].minY) / header->bitmapHeight;
							float32 maxX = (float32)(glyphs[i].maxX) / header->bitmapWidth;
							float32 minY = (float32)(glyphs[i].maxY) / header->bitmapHeight;

							s_Properties->fonts[storageIndex].glyphs[i].advance = glyphs[i].advance;
							s_Properties->fonts[storageIndex].glyphs[i].yOffset = glyphs[i].yOffset;
							s_Properties->fonts[storageIndex].atlasWidth = header->bitmapWidth;
							s_Properties->fonts[storageIndex].atlasHeight = header->bitmapHeight;
							s_Properties->fonts[storageIndex].numChars = header->numChars;
							s_Properties->fonts[storageIndex].atlasHandle = handle;

							uint16 regionHandle = TextureCreateRegion(handle, hpm::Vector2(minX, minY), hpm::Vector2(maxX, maxY));
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
					std::free(bitmapRGBA);
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

	void Renderer2D::DebugDrawString(hpm::Vector2 position, float32 scale, const char* string) {
		// TODO: This is all temporary
		// Here are gonna be direct submission to a drawQueue and sortBuffer
		float32 advance = position.x;
		Font* font = &s_Properties->fonts[DEFAULT_FONT_HANDLE - 1];
		for (uint32 at = 0; string[at]; at++) {
			// TODO: Standartize this - 32 thing with ASCII
			// Maybe just associate every glyph with ASCII code in font processor
			// and resolve all glyphs when loading font
			Glyph* glyph = &font->glyphs[(uint32)string[at] - 32];
			UV uv = GetTextureRegionUV(s_Properties, glyph->regionHandle);
			float32 width = (uv.max.x - uv.min.x) * font->atlasWidth * scale;
			float32 height = (uv.min.y - uv.max.y) * font->atlasHeight * scale;
			float32 yOffset = glyph->yOffset * scale;

			FillRectangleTexture(
				hpm::Vector2(advance, position.y - yOffset), 
				10, 0, 0, 
				hpm::Vector2(width, height), 
				glyph->regionHandle
			);
			advance += glyph->advance * scale;
		}
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
				AB_GLCALL(glGenTextures(1, &texHandle));
				AB_GLCALL(glBindTexture(GL_TEXTURE_2D, texHandle));

				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				AB_GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, inFormat, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.bitmap));

				AB_GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

				s_Properties->textures[freeIndex].used = true;
				s_Properties->textures[freeIndex].refCount = 1;
				s_Properties->textures[freeIndex].glHandle = texHandle;
				s_Properties->textures[freeIndex].format = image.format;
				s_Properties->textures[freeIndex].uv.min = hpm::Vector2(0.0f, 0.0f);
				s_Properties->textures[freeIndex].uv.max = hpm::Vector2(1.0f, 1.0f);
				s_Properties->textures[freeIndex].parent = 0;
				
				// NOTE: Increasing handle by one in order to use 0 value as invalid handle;
				resultHandle = freeIndex + 1;
	
				DeleteBitmap(image.bitmap);
			}
			else {
				AB_CORE_ERROR("Failed to load texture. Cannot load image: %s", filepath);
			}
		} else {
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

				AB_GLCALL(glGenTextures(1, &texHandle));
				AB_GLCALL(glBindTexture(GL_TEXTURE_2D, texHandle));

				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				AB_GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				AB_GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glGormat, GL_UNSIGNED_BYTE, bitmap));

				AB_GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

				s_Properties->textures[freeIndex].used = true;
				s_Properties->textures[freeIndex].refCount = 1;
				s_Properties->textures[freeIndex].glHandle = texHandle;
				s_Properties->textures[freeIndex].format = format;
				s_Properties->textures[freeIndex].uv.min = hpm::Vector2(0.0f, 0.0f);
				s_Properties->textures[freeIndex].uv.max = hpm::Vector2(1.0f, 1.0f);
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
				AB_GLCALL(glDeleteTextures(1, &s_Properties->textures[handle].glHandle));
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
		return s_Properties->textures[handle - 1].format;
	}

	void Renderer2D::FillRectangleColor(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color) {
		if (s_Properties->sortBufferLeftUsage != s_Properties->sortBufferRightUsage) {
			// Has alpha < 1.0f
			SortKey key = {};
			key.depth = depth;
			key.texHandle = 0;
			if ((color & 0xff000000) == 0xff000000) {
				s_Properties->drawQueue[s_Properties->drawQueueUsed] = { position, size,  angle, anchor, color, 0 };
				s_Properties->sortBufferA[s_Properties->sortBufferLeftUsage] = { key, s_Properties->drawQueueUsed};
				s_Properties->sortBufferLeftUsage++;
				s_Properties->drawQueueUsed++;
			}
			else {
				s_Properties->drawQueue[s_Properties->drawQueueUsed] = { position, size, angle, anchor, color, 0 };
				s_Properties->sortBufferA[s_Properties->sortBufferRightUsage] = { key, s_Properties->drawQueueUsed};
				s_Properties->sortBufferRightUsage--;
				s_Properties->drawQueueUsed++;
			}
		} else {
			AB_CORE_WARN("Failed to submit rectangle. Draw queue is full");
		}
	}

	void Renderer2D::FillRectangleTexture(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle) {
		if (s_Properties->sortBufferLeftUsage != s_Properties->sortBufferRightUsage) {
			uint16 baseTexHandle = GetTextureBaseHandle(s_Properties, textureHandle);
			SortKey key = {};
			key.depth = depth;
			key.texHandle = baseTexHandle;
			if (GetTextureFormat(baseTexHandle) == PixelFormat::RGB) {
				s_Properties->drawQueue[s_Properties->drawQueueUsed] = { position, size,  angle, anchor, 0, textureHandle };
				s_Properties->sortBufferA[s_Properties->sortBufferLeftUsage] = { key, s_Properties->drawQueueUsed};
				s_Properties->sortBufferLeftUsage++;
				s_Properties->drawQueueUsed++;
			}
			else {
				s_Properties->drawQueue[s_Properties->drawQueueUsed] = { position, size,  angle, anchor, 0, textureHandle };
				s_Properties->sortBufferA[s_Properties->sortBufferRightUsage] = { key, s_Properties->drawQueueUsed};
				s_Properties->sortBufferRightUsage--; 
				s_Properties->drawQueueUsed++;
			}
		}
		else {
			AB_CORE_WARN("Failed to submit rectangle. Draw queue is full");
		}
	}

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

			targetBuffer = left == bufferA ? bufferB: bufferA;

			uint32 leftAt = beg;
			uint32 rightAt = mid + 1;
			uint32 tempAt = beg;
			while (leftAt <= mid && rightAt <= end) {
				AB_CORE_ASSERT(tempAt <= end, "Merge sort error. Buffer owerflow.");
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

	inline static bool32 LeftSortPred(const SortKey* a, const SortKey* b) {
		return a->texHandle < b->texHandle;
	}

	inline static bool32 RightSortPred(const SortKey* a, const SortKey* b) {
		return a->value < b->value;
	}

	SortEntry* SortQueue(Renderer2DProperties* properties) {
		// NOTE: end is pointing to a last element
		//auto beg = __rdtsc();
		SortEntry* ptr = MergeSortV3(properties->sortBufferA, properties->sortBufferB, properties->sortBufferRightUsage + 1, Renderer2D::DRAW_QUEUE_CAPACITY - 1, RightSortPred);
		SortEntry* sortedBuffer = ptr;
		if (properties->sortBufferLeftUsage > 1) {
			SortEntry* bufferB = ptr == properties->sortBufferA ? properties->sortBufferB : properties->sortBufferA;
			sortedBuffer = MergeSortV3(ptr, bufferB, 0, properties->sortBufferLeftUsage - 1, LeftSortPred);
		}
		//uint64 end = __rdtsc();
		//uint64 elapsed = end - beg;

		//for (uint32 i = 0; i < Renderer2D::DRAW_QUEUE_CAPACITY; i++) {
		//	PrintString("%u16 %u16 %u32\n", sortedBuffer[i].key.depth, sortedBuffer[i].key.texHandle, sortedBuffer[i].renderQueueIndex);
		//}
		//PrintString("Sort time: %u64\n", elapsed);
		return  sortedBuffer;
	}

	static void GenVertexData(Renderer2DProperties* properties, SortEntry* entry) {
		// TODO: is there any elegant way to initialize non POD struct with zeros?
		UV uv;
		memset(&uv, 0, sizeof(UV));
		if (entry->key.texHandle != 0) {
			// TODO: There are two different handles now. Base handle in a sort entry and region handle in the draw queue
			uv = GetTextureRegionUV(properties, properties->drawQueue[entry->renderQueueIndex].regionTexHandle);
		}

		uint32 drawQueueIndex = entry->renderQueueIndex;
		float32 normalizedDepth = (float32)entry->key.depth / 20.0f;

		float32 sin = hpm::Sin(hpm::ToRadians(properties->drawQueue[drawQueueIndex].angle));
		float32 cos = hpm::Cos(hpm::ToRadians(properties->drawQueue[drawQueueIndex].angle));
		float32 invHalfW = (2 / properties->viewSpaceDim.x);
		float32 invHalfH = (2 / properties->viewSpaceDim.y);

		// rotation			// scale // translation	// ortho projection				   
		float32 originLBX = (((0 + properties->drawQueue[drawQueueIndex].anchor) * cos - (0 + properties->drawQueue[drawQueueIndex].anchor) * sin) * properties->drawQueue[drawQueueIndex].size.x + properties->drawQueue[drawQueueIndex].position.x) * invHalfW - 1;
		float32 originLBY = (((0 + properties->drawQueue[drawQueueIndex].anchor) * sin + (0 + properties->drawQueue[drawQueueIndex].anchor) * cos) * properties->drawQueue[drawQueueIndex].size.y + properties->drawQueue[drawQueueIndex].position.y) * invHalfH - 1;
		float32 originRBX = (((1 + properties->drawQueue[drawQueueIndex].anchor) * cos - (0 + properties->drawQueue[drawQueueIndex].anchor) * sin) * properties->drawQueue[drawQueueIndex].size.x + properties->drawQueue[drawQueueIndex].position.x) * invHalfW - 1;
		float32 originRBY = (((1 + properties->drawQueue[drawQueueIndex].anchor) * sin + (0 + properties->drawQueue[drawQueueIndex].anchor) * cos) * properties->drawQueue[drawQueueIndex].size.y + properties->drawQueue[drawQueueIndex].position.y) * invHalfH - 1;
		float32 originRTX = (((1 + properties->drawQueue[drawQueueIndex].anchor) * cos - (1 + properties->drawQueue[drawQueueIndex].anchor) * sin) * properties->drawQueue[drawQueueIndex].size.x + properties->drawQueue[drawQueueIndex].position.x) * invHalfW - 1;
		float32 originRTY = (((1 + properties->drawQueue[drawQueueIndex].anchor) * sin + (1 + properties->drawQueue[drawQueueIndex].anchor) * cos) * properties->drawQueue[drawQueueIndex].size.y + properties->drawQueue[drawQueueIndex].position.y) * invHalfH - 1;
		float32 originLTX = (((0 + properties->drawQueue[drawQueueIndex].anchor) * cos - (1 + properties->drawQueue[drawQueueIndex].anchor) * sin) * properties->drawQueue[drawQueueIndex].size.x + properties->drawQueue[drawQueueIndex].position.x) * invHalfW - 1;
		float32 originLTY = (((0 + properties->drawQueue[drawQueueIndex].anchor) * sin + (1 + properties->drawQueue[drawQueueIndex].anchor) * cos) * properties->drawQueue[drawQueueIndex].size.y + properties->drawQueue[drawQueueIndex].position.y) * invHalfH - 1;


		properties->vertexBuffer[properties->vertexCount].x = originLBX;
		properties->vertexBuffer[properties->vertexCount].y = originLBY;
		properties->vertexBuffer[properties->vertexCount].z = normalizedDepth;
		properties->vertexBuffer[properties->vertexCount].color = properties->drawQueue[drawQueueIndex].color;
		properties->vertexBuffer[properties->vertexCount].u = uv.min.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.min.y;
		properties->vertexCount++;
		//right bottom
		properties->vertexBuffer[properties->vertexCount].x = originRBX;
		properties->vertexBuffer[properties->vertexCount].y = originRBY;
		properties->vertexBuffer[properties->vertexCount].z = normalizedDepth;
		properties->vertexBuffer[properties->vertexCount].color = properties->drawQueue[drawQueueIndex].color;
		properties->vertexBuffer[properties->vertexCount].u = uv.max.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.min.y;
		properties->vertexCount++;
		//right top
		properties->vertexBuffer[properties->vertexCount].x = originRTX;
		properties->vertexBuffer[properties->vertexCount].y = originRTY;
		properties->vertexBuffer[properties->vertexCount].z = normalizedDepth;
		properties->vertexBuffer[properties->vertexCount].color = properties->drawQueue[drawQueueIndex].color;
		properties->vertexBuffer[properties->vertexCount].u = uv.max.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.max.y;
		properties->vertexCount++;
		// left top
		properties->vertexBuffer[properties->vertexCount].x = originLTX;
		properties->vertexBuffer[properties->vertexCount].y = originLTY;
		properties->vertexBuffer[properties->vertexCount].z = normalizedDepth;
		properties->vertexBuffer[properties->vertexCount].color = properties->drawQueue[drawQueueIndex].color;
		properties->vertexBuffer[properties->vertexCount].u = uv.min.x;
		properties->vertexBuffer[properties->vertexCount].v = uv.max.y;
		properties->vertexCount++;

		properties->indexCount += 6;
	}

	void GenVertexAndBatchBuffers(Renderer2DProperties* properties, SortEntry* sortedBuffer) {
		uint32 batchCount = 0;
		for (uint64 i = 0; i < properties->sortBufferLeftUsage; i++) {
			if (sortedBuffer[i].key.depth < sortedBuffer[Renderer2D::DRAW_QUEUE_CAPACITY - 1].key.depth) {
				batchCount++;
				// Check for last sprite
				if (i + 1 == properties->sortBufferLeftUsage) {
					properties->batches[properties->batchesUsed].count = batchCount;
					properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;

					properties->batchesUsed++;
					batchCount = 0;
				}
				else {
					if (sortedBuffer[i].key.texHandle != sortedBuffer[i + 1].key.texHandle) {
						properties->batches[properties->batchesUsed].count = batchCount;
						properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;
						properties->batchesUsed++;
						batchCount = 0;
					}
				}

				GenVertexData(properties, &sortedBuffer[i]);
			}
		}
		//batchCount = 0;
		for (uint64 i = properties->sortBufferRightUsage + 1; i < Renderer2D::DRAW_QUEUE_CAPACITY; i++) {
			batchCount++;
			// Check for last sprite
			if (i + 1 == Renderer2D::DRAW_QUEUE_CAPACITY) {
				properties->batches[properties->batchesUsed].count = batchCount;
				properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;

				properties->batchesUsed++;
				batchCount = 0;
			}
			else {
				if (sortedBuffer[i].key.texHandle != sortedBuffer[i + 1].key.texHandle) {
					properties->batches[properties->batchesUsed].count = batchCount;
					properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;
					properties->batchesUsed++;
					batchCount = 0;
				}
			}

			GenVertexData(properties, &sortedBuffer[i]);
		}

		for (uint64 i = 0; i < properties->sortBufferLeftUsage; i++) {
			if (sortedBuffer[i].key.depth >= sortedBuffer[Renderer2D::DRAW_QUEUE_CAPACITY - 1].key.depth) {
				batchCount++;
				// Check for last sprite
				if (i + 1 == properties->sortBufferLeftUsage) {
					properties->batches[properties->batchesUsed].count = batchCount;
					properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;

					properties->batchesUsed++;
					batchCount = 0;
				}
				else {
					if (sortedBuffer[i].key.texHandle != sortedBuffer[i + 1].key.texHandle) {
						properties->batches[properties->batchesUsed].count = batchCount;
						properties->batches[properties->batchesUsed].textureHandle = sortedBuffer[i].key.texHandle;
						properties->batchesUsed++;
						batchCount = 0;
					}
				}

				GenVertexData(properties, &sortedBuffer[i]);
			}
		}
	}

	static void ResetRenderState(Renderer2DProperties* properties) {
		properties->batchesUsed = 0;
		properties->vertexCount = 0;
		properties->indexCount = 0;
		properties->sortBufferLeftUsage = 0;
		properties->sortBufferRightUsage = Renderer2D::DRAW_QUEUE_CAPACITY - 1;
		properties->drawQueueUsed = 0;
	}

	void Renderer2D::Flush() {
		SortEntry* sortedBuffer = SortQueue(s_Properties);
		GenVertexAndBatchBuffers(s_Properties, sortedBuffer);

		AB_GLCALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
		AB_GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		AB_GLCALL(glBindBuffer(GL_ARRAY_BUFFER, s_Properties->GLVBOHandle));
		AB_GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * s_Properties->vertexCount, (void*)s_Properties->vertexBuffer, GL_DYNAMIC_DRAW));
		AB_GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Properties->GLIBOHandle));

		AB_GLCALL(glEnableVertexAttribArray(0));
		AB_GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));
		AB_GLCALL(glEnableVertexAttribArray(1));
		AB_GLCALL(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), (void*)(sizeof(float32) * 3)));
		AB_GLCALL(glEnableVertexAttribArray(2));
		AB_GLCALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(float32) * 3 + sizeof(float) * 1)));

		AB_GLCALL(glUseProgram(s_Properties->shaderHandle));
		AB_GLCALL(glUniform1i(glGetUniformLocation(s_Properties->shaderHandle, "tex"), 0));
		uint32 drawCalls = 0;
		uint64 verticesDrawn = 0;
		for (uint64 i = 0; i < s_Properties->batchesUsed; i++) {
			AB_GLCALL(glUniform1f(glGetUniformLocation(s_Properties->shaderHandle, "useTexture"), s_Properties->batches[i].textureHandle ? 1.0f : 0.0f));
			AB_GLCALL(glActiveTexture(GL_TEXTURE0));
			if (s_Properties->batches[i].textureHandle > 0) {
				AB_GLCALL(glBindTexture(GL_TEXTURE_2D, GetTextureRegionAPIHandle(s_Properties, s_Properties->batches[i].textureHandle)));
			}
			
			AB_GLCALL(glDrawElements(GL_TRIANGLES, 6 * s_Properties->batches[i].count, GL_UNSIGNED_SHORT, (void*)verticesDrawn));
			drawCalls++;
			verticesDrawn += 6 * s_Properties->batches[i].count * sizeof(uint16);
		}

		ResetRenderState(s_Properties);

		system("cls");
		AB::PrintString("DC: %u32\n", drawCalls);
	}

	void _GLInit(Renderer2DProperties* properties) {
		uint32 winWidth;
		uint32 winHeight;
		Window::GetSize(winWidth, winHeight);
		AB_GLCALL(glViewport(0, 0, winWidth, winHeight));

		AB_GLCALL(glGenBuffers(1, &properties->GLVBOHandle));

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

		AB_GLCALL(glGenBuffers(1, &properties->GLIBOHandle));
		AB_GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, properties->GLIBOHandle));
		AB_GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16) * Renderer2D::INDEX_BUFFER_SIZE, indices, GL_STATIC_DRAW));
		AB_GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		std::free(indices);

		int32 vertexShader;
		AB_GLCALL(vertexShader = glCreateShader(GL_VERTEX_SHADER));
		AB_GLCALL(glShaderSource(vertexShader, 1, &VERTEX_SOURCE, 0));
		AB_GLCALL(glCompileShader(vertexShader));

		int32 fragmentShader;
		AB_GLCALL(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
		AB_GLCALL(glShaderSource(fragmentShader, 1, &FRAGMENT_SOURCE, 0));
		AB_GLCALL(glCompileShader(fragmentShader));

		int32 result = 0;
		AB_GLCALL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result));
		if (!result) {
			int32 logLen;
			AB_GLCALL(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			AB_GLCALL(glGetShaderInfoLog(vertexShader, logLen, NULL, message));
			AB_CORE_FATAL("Shader compilation error:\n%s", message);
		};

		AB_GLCALL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result));
		if (!result) {
			int32 logLen;
			AB_GLCALL(glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			AB_GLCALL(glGetShaderInfoLog(fragmentShader, logLen, NULL, message));
			AB_CORE_FATAL("Shader compilation error:\n%s", message);
		};

		AB_GLCALL(properties->shaderHandle = glCreateProgram());
		AB_GLCALL(glAttachShader(properties->shaderHandle, vertexShader));
		AB_GLCALL(glAttachShader(properties->shaderHandle, fragmentShader));
		AB_GLCALL(glLinkProgram(properties->shaderHandle));
		AB_GLCALL(glGetProgramiv(properties->shaderHandle, GL_LINK_STATUS, &result));
		if (!result) {
			int32 logLen;
			AB_GLCALL(glGetProgramiv(properties->shaderHandle, GL_INFO_LOG_LENGTH, &logLen));
			char* message = (char*)alloca(logLen);
			AB_GLCALL(glGetProgramInfoLog(properties->shaderHandle, logLen, 0, message));
			AB_CORE_FATAL("Shader compilation error:\n%s", message);
		}

		AB_GLCALL(glDeleteShader(vertexShader));
		AB_GLCALL(glDeleteShader(fragmentShader));
	}


	void RenderGroupResizeCallback(uint32 width, uint32 height) {
		if (Renderer2D::s_Properties) {

			AB_GLCALL(glViewport(0, 0, width, height));
		}
	}
}
