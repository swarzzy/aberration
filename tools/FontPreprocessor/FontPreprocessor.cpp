#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cassert>

typedef int8_t          int8;
typedef int16_t	        int16;
typedef int32_t	        int32;
typedef int64_t	        int64;
typedef uint8_t	        uint8;
typedef uint16_t        uint16;
typedef uint32_t        uint32;
typedef uint64_t        uint64;
typedef uint32          bool32;
typedef char*           char8;
typedef unsigned char   uchar8;
typedef uint8_t         byte;
typedef float           float32;
typedef double          float64;
typedef uintptr_t       uintptr;

#if defined(AB_PLATFORM_WINDOWS)
#include <windows.h>

void DebugFreeFileMemory(void* memory) {
	if (memory) {
		std::free(memory);
	}
}

void* DebugReadFile(const char* filename, uint32* bytesRead) {
	*bytesRead = 0;
	void* bitmap = nullptr;
	LARGE_INTEGER fileSize = {0};
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			if (fileSize.QuadPart > 0xffffffff) {
				printf("Can`t read >4GB file.");
				CloseHandle(fileHandle);
				return nullptr;
			}
			bitmap = std::malloc(fileSize.QuadPart);
			if (bitmap) {
				DWORD read;
				if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
					printf("Failed to read file.");
					DebugFreeFileMemory(bitmap);
					bitmap = nullptr;
				}
			}
		}
		CloseHandle(fileHandle);
	}
	*bytesRead = (uint32)fileSize.QuadPart;
	return  bitmap;
}

bool32 DebugWriteFile(const char* filename, void* data, uint32 dataSize) {
	HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		if (WriteFile(fileHandle, data, dataSize, &bytesWritten, 0) && (dataSize == bytesWritten)) {
			CloseHandle(fileHandle);
			return true;
		}
	}
	CloseHandle(fileHandle);
	return false;
}

#elif defined(AB_PLATFORM_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void DebugFreeFileMemory(void* memory) {
	if (memory) {
		std::free(memory);
	}
}

void* DebugReadFile(const char* filename, uint32* bytesRead) {
	*bytesRead = 0;
	void* ptr = nullptr;
	*bytesRead = 0;
	int fileHandle = open(filename, O_RDONLY);
	if (fileHandle) {
		off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
		if (fileEnd) {
			lseek(fileHandle, 0, SEEK_SET);
			void* data = std::malloc(fileEnd);
			if (data) {
				ssize_t result = read(fileHandle, data, fileEnd);
				if (result == fileEnd) {
					ptr = data;
					*bytesRead = (uint32)result;
				}
				else {
					std::free(data);
					printf("File reading error. File: %s. Failed read data from file", filename);
				}
			}
			else {
				printf("File reading error. File: %s. Memory allocation failed", filename);
			}
		}
		else {
			printf("File reading error. File: %s", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file.", filename);
		return ptr;
	}
	close(fileHandle);
	return ptr;
	}

bool32 DebugWriteFile(const char* filename, void* data, uint32 dataSize) {
	bool32 result = false;
	int fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IRWXU | S_IRGRP);
	if (fileHandle) {
		ssize_t written = write(fileHandle, data, dataSize);
		if (written == dataSize) {
			result = true;
		}
		else {
			printf("File reading error. File: %s. Write operation failed.", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file", filename);
		return false;
	}
	close(fileHandle);
	return result;
}
#else
#error Unsupported platform.
#endif

constexpr char HELP_MESSAGE[] = R"(
Usage:
FontPreprocessor.exe [FLAG]... [INPUT FILE]
Flags:
-h : help
-x : bitmap width
-y : bitmap height
-f : font height
-b : number of the first char (ASCII code)
-c : number of chars to handle
-o : output file name.
)";

#define MAX_BITMAP_WIDTH 4096
#define MAX_BITMAP_HEIGHT 4096
#define MAX_FONT_HEIGHT 256
#define MAX_FIRST_CHAR_NUM 0xffff // Unicode - 2 bytes

#define DEFAULT_BITMAP_WIDTH 512
#define DEFAULT_BITMAP_HEIGHT 512
#define DEFAULT_FONT_HEIGHT 64
#define DEFAULT_FIRST_CHAR_NUMBER 32
#define DEFAULT_NUM_CHARS 96
#define DEFAULT_FILENAME "font.bitmap"

struct CommandLineArgs {
    bool32 isHelpQuery;
    int32 width;
    int32 height;
    int32 fontHeight;
    int32 firstChar;
    int32 numChars;
    const char* filepath;
	const char* filename;
};

enum class ArgType {
    Unknown = 0,
    BitmapWidth,
    BitmapHeight,
    FontHeight,
    FirstChar,
    NumChars,
    Filepath,
    HelpQuery
};

static CommandLineArgs ParseCommandLineArgs(int argc, char** argv) {
    CommandLineArgs parameters = {};
    int32 argvIndex = 1;
    if (argc > 1) {
        while (argvIndex < argc) {
            const char* arg = argv[argvIndex];
            if (arg[0] == '-') {
                switch(arg[1]) {
                    case 'h': { // help query
                        parameters.isHelpQuery = true;
                        return parameters;
                    } break;
                    case 'x': { // bitmap width
                        int32 intArg = std::atoi(&arg[2]);
                        if (intArg > 0 && intArg < MAX_BITMAP_WIDTH) {
                            parameters.width = intArg;
                        } else {
                            printf("Incorrect bitmap width: %d. Using default width: %dx\n",
                                    intArg, DEFAULT_BITMAP_WIDTH);
                                    parameters.width = DEFAULT_BITMAP_WIDTH;
                        }
                    } break;
					case 'y': { // bitmap height
					    int32 intArg = std::atoi(&arg[2]);
					    if (intArg > 0 && intArg < MAX_BITMAP_HEIGHT) {
					        parameters.height = intArg;
					    } else {
					        printf("Incorrect bitmap height: %d. Using default height: %dx\n",
					        intArg, DEFAULT_BITMAP_HEIGHT);
					        parameters.height = DEFAULT_BITMAP_HEIGHT;
					    }
					} break;
					case 'f': { // bitmap height
					    int32 intArg = std::atoi(&arg[2]);
					    if (intArg > 0 && intArg < MAX_FONT_HEIGHT) {
					        parameters.fontHeight = intArg;
					    } else {
					        printf("Incorrect font height: %d. Using default height: %dx\n",
					        intArg, DEFAULT_FONT_HEIGHT);
					        parameters.fontHeight = DEFAULT_FONT_HEIGHT;
					    }
					} break;
					case 'b': { // first char
					    int32 intArg = std::atoi(&arg[2]);
					    if (intArg > 0 && intArg < MAX_FIRST_CHAR_NUM) {
					        parameters.firstChar = intArg;
					    } else {
					        printf("Incorrect first character number: %d. Using default first character number: %dx\n",
					        intArg, DEFAULT_FIRST_CHAR_NUMBER);
					        parameters.firstChar = DEFAULT_FIRST_CHAR_NUMBER;
					    }
					} break;
					case 'c': { // chars number
					    int32 intArg = std::atoi(&arg[2]);
					    // NOTE: This value might be bigger than it should be
					    // Hese is not actually correct checking
					    if (intArg > 0 && intArg < MAX_FIRST_CHAR_NUM) {
					        parameters.numChars = intArg;
					    } else {
					        printf("Incorrect character number: %d. Using default character number: %dx\n",
					        intArg, DEFAULT_NUM_CHARS);
					        parameters.numChars = DEFAULT_NUM_CHARS;
					    }
					} break;
					case 'o': { // out file name
						const char* str = &arg[2];
						parameters.filename = str;
					} break;
					default: {
					    printf("Unknown argument: %s\n", arg);
					} break;
                }
                        argvIndex++;
            } else {
                const char* filepath = argv[argvIndex];
                parameters.filepath = filepath;
                break;
            }
        }
    } else {
        printf("No input files!\n");
    }

    if (parameters.width <= 0) {
        printf("Bitmap width not specified. Using deafult width: %d\n",
                DEFAULT_BITMAP_WIDTH);
        parameters.width = DEFAULT_BITMAP_WIDTH;
    }
    if (parameters.height <= 0) {
        printf("Bitmap height not specified. Using deafult height: %d\n",
                DEFAULT_BITMAP_HEIGHT);
        parameters.height = DEFAULT_BITMAP_HEIGHT;
    }

	if (parameters.firstChar <= 0) {
		printf("First character number not specified. Using deafult number: %d\n",
			DEFAULT_FIRST_CHAR_NUMBER);
		parameters.firstChar = DEFAULT_FIRST_CHAR_NUMBER;
	}
	if (parameters.numChars <= 0) {
		printf("Number of characters not specified. Using deafult number: %d\n",
			DEFAULT_NUM_CHARS);
		parameters.numChars = DEFAULT_NUM_CHARS;
	}

    if (parameters.fontHeight <= 0) {
        printf("Font height not specified. Using deafult font height: %d\n",
                DEFAULT_FONT_HEIGHT);
        parameters.fontHeight = DEFAULT_FONT_HEIGHT;
    }

	if (!parameters.filename) {
		parameters.filename = DEFAULT_FILENAME;
	}

    return parameters;
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
	float32 xOffset;
	float32 yOffset;
	float32 advance;
};
#pragma pack(pop)

#define zero_array(begin, array_count) (memset(begin, 0, sizeof(begin) * array_count))
#define zero_memory(begin) (memset(begin, 0, sizeof(begin)))
#define zero_allocate(type, count) ((type*)memset(malloc(sizeof(type) * count), 0, sizeof(type) * count))

int main(int argc, char** argv) {
	CommandLineArgs args = ParseCommandLineArgs(argc, argv);
	uint32 bytesRead = 0;
#if defined(AB_PLATFORM_WINDOWS)
	const char* path = "c:\\windows\\fonts\\SourceCodePro-Regular.ttf";
#else
	const char* path = nullptr;
#endif
	if (args.filepath) {
		path = args.filepath;
	}
	byte* fileData = (byte*)DebugReadFile(path, &bytesRead);

	if (fileData) {
		float32 fontSize = 32;
		uint32 cyrillicNumChars = 0x044f - 0x0410 + 1;
		uint32 asciiNumChars = 96;
		uint32 totalNumChars = cyrillicNumChars + asciiNumChars;

		// | header | glyph data | kerning table | bitmap |
		uint32 bitmapWidth = 512;
		uint32 bitmapSize = sizeof(byte) * bitmapWidth * bitmapWidth;
		uint32 kernTableSize = sizeof(int16) * totalNumChars * totalNumChars;
		uint32 bitmapOffset = sizeof(ABFontBitmapHeader) + sizeof(PackedGlyphData) * totalNumChars + kernTableSize;
		uint32 glyphDataSize = sizeof(PackedGlyphData) * totalNumChars;
		uint32 kernTableOffset = sizeof(ABFontBitmapHeader) + sizeof(PackedGlyphData) * totalNumChars;
		uint32 fileSize = sizeof(ABFontBitmapHeader) + glyphDataSize + kernTableSize + bitmapSize;

		byte* out = zero_allocate(byte, fileSize);

		byte* bitmap = out + bitmapOffset;

		stbtt_fontinfo font;
		int fontIndex = stbtt_GetFontOffsetForIndex(fileData, 0);
		stbtt_pack_context context;
		//stbtt_PackSetOversampling(&context, 2, 2);
		stbtt_PackBegin(&context, bitmap, bitmapWidth, bitmapWidth, 0, 1, nullptr);

		stbtt_packedchar* ASCIIChars = {};
		stbtt_packedchar* cyrillicChars = {};

		ASCIIChars = zero_allocate(stbtt_packedchar, asciiNumChars);
		cyrillicChars = zero_allocate(stbtt_packedchar, asciiNumChars);

		stbtt_pack_range ranges[2];
		memset(ranges, 0, sizeof(stbtt_pack_range) * 2);

		ranges[0].font_size = fontSize;
		ranges[0].first_unicode_codepoint_in_range = 32;
		ranges[0].num_chars = asciiNumChars;
		ranges[0].chardata_for_range = ASCIIChars;

		ranges[1].font_size = fontSize;
		ranges[1].first_unicode_codepoint_in_range = 0x0410;
		ranges[1].num_chars = cyrillicNumChars;
		ranges[1].chardata_for_range = cyrillicChars;

		stbtt_PackFontRanges(&context, fileData, fontIndex, ranges, 2);

		stbtt_PackEnd(&context);

		stbtt_InitFont(&font, fileData, fontIndex);
		float32 scaleFactor = stbtt_ScaleForPixelHeight(&font, fontSize);

		int ascent = 0;
		int descent = 0;
		int lineGap = 0;
		stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
		float32 lineAdvance = (ascent - descent + lineGap) * scaleFactor;

		ABFontBitmapHeader* header = (ABFontBitmapHeader*)out;
		header->format = AB_FONT_BITMAP_FORMAT_KEY;
		header->scaleFactor = scaleFactor;
		header->bitmapBeginOffset = bitmapOffset;
		header->bitmapHeight = bitmapWidth;
		header->bitmapWidth = bitmapWidth;
		header->heightInPixels = fontSize;
		header->kernTableOffset = kernTableOffset;
		header->lineAdvance = lineAdvance;
		header->numCodepoints = totalNumChars;
		header++;

		PackedGlyphData* glyph = (PackedGlyphData*)header;
		PackedGlyphData* glyphDataBegin = (PackedGlyphData*)header;


		uint32 unicodeCodepoint = 32;
		for (uint32 i = 0; i < asciiNumChars; i++) {
			// NOTE: Shuffling y because an atlas is top-down pixel order
			// but engine uses down-to-up pixel order
			glyph->unicodeCodepoint = unicodeCodepoint;
			glyph->minX = ASCIIChars[i].x0;
			glyph->minY = ASCIIChars[i].y1;
			glyph->maxX = ASCIIChars[i].x1;
			glyph->maxY = ASCIIChars[i].y0;
			glyph->xOffset = ASCIIChars[i].xoff;
			glyph->yOffset = ASCIIChars[i].yoff;
			glyph->advance = ASCIIChars[i].xadvance;
			unicodeCodepoint++;
			glyph++;
		}

		unicodeCodepoint = 0x0410;
		for (uint32 i = 0; i < cyrillicNumChars; i++) {
			glyph->unicodeCodepoint = unicodeCodepoint;
			glyph->minX = cyrillicChars[i].x0;
			glyph->minY = cyrillicChars[i].y1;
			glyph->maxX = cyrillicChars[i].x1;
			glyph->maxY = cyrillicChars[i].y0;
			glyph->xOffset = cyrillicChars[i].xoff;
			glyph->yOffset = cyrillicChars[i].yoff;
			glyph->advance = cyrillicChars[i].xadvance;
			unicodeCodepoint++;
			glyph++;
		}

		int16 * kernTableFile = (int16*)glyph;
		int16 * kernTableFileAt = (int16*)glyph;


		// NOTE: Array indexing order : numChars * first + second
		for (uint32 i = 0; i < totalNumChars; i++) {
			for (uint32 j = 0; j < totalNumChars; j++) {
				uint32 first = glyphDataBegin[i].unicodeCodepoint;
				uint32 second = glyphDataBegin[j].unicodeCodepoint;
				kernTableFile[asciiNumChars * i + j] = (int16)stbtt_GetCodepointKernAdvance(&font, first, second);
				kernTableFileAt++;
			}
		}
		assert((byte*)kernTableFileAt == (byte*)(out + bitmapOffset));

		DebugWriteFile("arial.abf", out, (uint32)fileSize);
	}
}

#if 0
	int result = 0;
	CommandLineArgs args = ParseCommandLineArgs(argc, argv);
	if (args.isHelpQuery) {
	    printf("%s", HELP_MESSAGE);
	} else {
	    uint32 bytesRead = 0;
	    byte* fileData = (byte*)DebugReadFile(args.filepath, &bytesRead);
	    if (fileData && bytesRead) {
			//
			uint32 cyrillicNumChars = 0x044f - 0x0410;
			//
			uint32 bitmapSize = sizeof(byte) * args.width * args.height;
			uint32 kernTableSize = sizeof(int16) * (args.numChars + cyrillicNumChars) * (args.numChars + cyrillicNumChars);
			uint32 bitmapOffset = sizeof(ABFontBitmapHeader) + sizeof(PackedGlyphData) * (args.numChars + cyrillicNumChars) + kernTableSize;
			uint32 kernTableOffset = sizeof(ABFontBitmapHeader) + sizeof(PackedGlyphData) * (args.numChars + cyrillicNumChars);
			uint32 fileSize = bitmapOffset + bitmapSize + kernTableSize;
			byte* outFileData = (byte*)std::malloc(fileSize);
			memset(outFileData, 0, fileSize);
			//zero_array(outFileData, fileSize);
			if (outFileData) {
				byte* bitmap = outFileData + bitmapOffset;

				stbtt_fontinfo font;
				int fontIndex = stbtt_GetFontOffsetForIndex(fileData, 0);
				// NOTE: Initialization isn't needed for stbtt_BakeFontBitmap
				// It's used only for getting metrics
				if (stbtt_InitFont(&font, fileData, fontIndex)) {

					stbtt_pack_context context;
					result = stbtt_PackBegin(
						&context, 
						bitmap, 
						args.width, 
						args.height, 
						0,
						1,
						nullptr
					);
					stbtt_packedchar* ASCIIChars = {};
					stbtt_packedchar* cyrillicChars = {};

					
					ASCIIChars = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * args.numChars);
					cyrillicChars = (stbtt_packedchar*)malloc(sizeof(stbtt_packedchar) * cyrillicNumChars);

					stbtt_pack_range ranges[2];
					//zero_array(ranges, 2);
					memset(ranges, 0, sizeof(stbtt_pack_range) * 2);

					ranges[0].font_size = (float32)args.fontHeight;
					ranges[0].first_unicode_codepoint_in_range = args.firstChar;
					ranges[0].num_chars = args.numChars;
					ranges[0].chardata_for_range = ASCIIChars;

					ranges[1].font_size = (float32)args.fontHeight;
					ranges[1].first_unicode_codepoint_in_range = 0x0410;
					ranges[1].num_chars = cyrillicNumChars;
					ranges[1].chardata_for_range = cyrillicChars;
					stbtt_PackFontRanges(&context, fileData, fontIndex, ranges, 2);

					stbtt_PackEnd(&context);


					//stbtt_bakedchar* characters = (stbtt_bakedchar*)std::malloc(sizeof(stbtt_bakedchar) * args.numChars);
					//result = stbtt_BakeFontBitmap(
					//	(byte*)fileData,
					//	0,
					//	(float)args.fontHeight,
					//	bitmap,
					//	args.width,
					//	args.height,
					//	args.firstChar,
					//	args.numChars,
					//	characters
					//);

#if 0
					// Flip bitmap vertically
					for (uint64 i = 0; i < (bitmapSize / 2); i++) {
						uint64 j = bitmapSize - i - 1;
						byte tmp = bitmap[j];
						bitmap[j] = bitmap[i];
						bitmap[i] = tmp;
					}
					
					// Flip bitmap horizontally
					for (uint64 i = 0; i < bitmapSize; i += args.width) {
						uint64 k = i + args.width - 1;
						for (uint64 j = i; j < (i + args.width / 2); j++) {
							byte tmp = bitmap[k];
							bitmap[k] = bitmap[j];
							bitmap[j] = tmp;
							k--;
						}
					}
#endif									
					//
					//int ascent = 0;
					//int descent = 0;
					

					float32 scaleFactor = stbtt_ScaleForPixelHeight(&font, (float32)args.fontHeight);

					int ascent = 0;
					int descent = 0;
					int lineGap = 0;
					stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

					float32 lineAdvance = (ascent - descent + lineGap) * scaleFactor;

					ABFontBitmapHeader* header = (ABFontBitmapHeader*)outFileData;
					header->format = AB_FONT_BITMAP_FORMAT_KEY;
					// FIXME: Bad conversion
					header->heightInPixels = (float32)args.fontHeight;
					//header->firstCodePoint = args.firstChar;
					header->numCodepoints = args.numChars + cyrillicNumChars;
					header->bitmapBeginOffset = bitmapOffset;
					header->bitmapWidth = args.width;
					header->bitmapHeight = args.height;
					header->kernTableOffset = kernTableOffset;
					header->scaleFactor = scaleFactor;
					header->lineAdvance = lineAdvance;
					//header->fontAscent = ascent * scaleFactor;
					//header->fontDescent = descent * scaleFactor;
					header++;
					PackedGlyphData* glyph = (PackedGlyphData*)header;
					// Packing PackedGlyphData array
					int32 unicodeCodepoint = args.firstChar;
					for (int32 i = 0; i < args.numChars; i++) {
						// NOTE: Shuffling y because an atlas is top-down pixel order
						// but engine uses down-to-up pixel order
						glyph->unicodeCodepoint = (wchar_t)unicodeCodepoint;
						glyph->minX = ASCIIChars[i].x0;
						glyph->minY = ASCIIChars[i].y1;
						glyph->maxX = ASCIIChars[i].x1;
						glyph->maxY = ASCIIChars[i].y0;
						glyph->xOffset = ASCIIChars[i].xoff;
						glyph->yOffset = ASCIIChars[i].yoff;
						//printf("%d, %c, %f\n", i, (char)(i + 32), characters[i].xadvance);
						glyph->advance = ASCIIChars[i].xadvance;
						unicodeCodepoint++;
						glyph++;
					}

					unicodeCodepoint = 0x0410;
					//
					for (uint32 i = 0; i < cyrillicNumChars; i++) {
						// NOTE: Shuffling y because an atlas is top-down pixel order
						// but engine uses down-to-up pixel order
						glyph->unicodeCodepoint = (wchar_t)unicodeCodepoint;
						glyph->minX = cyrillicChars[i].x0;
						glyph->minY = cyrillicChars[i].y1;
						glyph->maxX = cyrillicChars[i].x1;
						glyph->maxY = cyrillicChars[i].y0;
						glyph->xOffset = cyrillicChars[i].xoff;
						glyph->yOffset = cyrillicChars[i].yoff;
						//printf("%d, %c, %f\n", i, (char)(i + 32), characters[i].xadvance);
						glyph->advance = cyrillicChars[i].xadvance;
						unicodeCodepoint++;
						glyph++;
					}
					//

					int16 * kernTableFileAt = (int16*)glyph;
					// NOTE: Array indexing order : numChars * first + second
					for (int32 i = args.firstChar; i < args.numChars + args.firstChar; i++) {
						for (int32 j = args.firstChar; j < args.numChars + args.firstChar; j++) {
							int32 first = i - args.firstChar;
							int32 second = j - args.firstChar;
							//printf("%u %c %c %d\n",(uint32)(args.numChars * first + second), (char)i, (char)j, stbtt_GetCodepointKernAdvance(&font, i, j));
							kernTableFileAt[args.numChars * first + second] = (int16)stbtt_GetCodepointKernAdvance(&font, i, j);
						}
					}

					//assert((byte*)glyph == (byte*)(outFileData + bitmapOffset));

					DebugWriteFile(args.filename, outFileData, (uint32)fileSize);
				} else {
					printf("Failed to load font from file: %s", args.filename);
				}
			}
	    }
	}
	if (result < 1) {
		printf("Not all of the specified characters fit into the bitmap. Number of characters that fit: %d", result * -1);
	}
  return 0;
}
#endif