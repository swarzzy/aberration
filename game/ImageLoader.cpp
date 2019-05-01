#include "ImageLoader.h"
#include "Log.h"
#include <hypermath.h>

#include <cstring>

namespace AB {
	
	Image LoadBMP(MemoryArena* memory, const char* filename, ColorSpace cs) {
		Image image = {};
		uint32 dataSize;
		byte* data = (byte*)DebugReadFilePermanent(memory, filename, &dataSize);
		if (!data) {
			AB_CORE_WARN("Failed to load BMP image: %s. Failed to open file.", filename);
			return image;
		}

		BMPHeader* header = (BMPHeader*)data;
		BMPInfoHeaderCore* infoHeader = (BMPInfoHeaderCore*)(data + sizeof(BMPHeader));
		// TODO: Check is it actually bmp
		uint16 bitsPerPixel = 0;

		bool32 bottomUp = true;
		switch (infoHeader->structSize) {
			case sizeof(BMPInfoHeaderCore) : {
				// Core version
				image.width = infoHeader->width;
				image.height = infoHeader->height;
				bitsPerPixel = infoHeader->bitsPerPixel;
			} break;
				case sizeof(BMPInfoHeaderV3) : {
					// Version 3
					BMPInfoHeaderV3* v3 = (BMPInfoHeaderV3*)infoHeader;
					if (infoHeader->height < 0) {
						bottomUp = false;
						image.height = (uint32)hpm::Abs(v3->height);
					}
					else {
						image.height = v3->height;
					}
					image.width = v3->width;
					bitsPerPixel = v3->bitsPerPixel;

				} break;
					case sizeof(BMPInfoHeaderV4) : {
						// Version 4
						// TODO: masks
						BMPInfoHeaderV4* v4 = (BMPInfoHeaderV4*)infoHeader;
						if (v4->height < 0) {
							bottomUp = false;
							image.height = (uint32)hpm::Abs(v4->height);
						}
						else {
							image.height = v4->height;
						}
						image.width = v4->width;
						bitsPerPixel = v4->bitsPerPixel;
					} break;
					default: {
						if (infoHeader->structSize >= 12) {
							// Assume that version is above than core so dims are signed
							BMPInfoHeaderV3Cut* v3c = (BMPInfoHeaderV3Cut*)infoHeader;
							//
							// TODO: Check if V3 header actually gives correct representation of data
							// Size if main header is not equals number of pixels. 
							//(It also looks like that this size in main header actually not equals file size neither)
							//
							//if (((uint32)(std::abs(v3c->height) * v3c->width)) == (header->size / (v3c->bitsPerPixel / 8))) {
								if (v3c->height < 0) {
									bottomUp = false;
									image.height = (uint32)std::abs(v3c->height);
								}
								else {
									image.height = v3c->height;
								}
								image.width = v3c->width;
								bitsPerPixel = v3c->bitsPerPixel;
							//}
							AB_CORE_WARN("WARNING: Unknown version of BMP header in file: %s", filename);
							//memset(&image, 0, sizeof(Image));
							//AB::DebugFreeFileMemory(data);
							//return image;

						}
						else {
							AB_CORE_WARN("Failed to load BMP image: %s. Unknown format version", filename);
							memset(&image, 0, sizeof(Image));
							// TODO: use stack and deallocate
							//AB::DebugFreeFileMemory(data);
							return image;
						}
					} break;
		}

		// TODO: TEMPORARY: Abort loading if pixel order is top->down 
		if (!bottomUp) {
			AB_CORE_WARN("Failed to load BMP image: %s. Wrong data order.", filename);
			memset(&image, 0, sizeof(Image));
			// TODO: use stack and deallocate			
			//AB::DebugFreeFileMemory(data);
			return image;
		}

		image.bitmap = (byte*)data + header->offsetToBitmap;
		// TODO: TEMPORARY: Store pointer to a beginning of block right before bitmap
		*((uintptr*)image.bitmap - 1) = (uintptr)data;

		if (bitsPerPixel == 32) {
			if (cs == TEX_COLOR_SPACE_SRGB) {
				image.format = TEX_FORMAT_SRGB8_A8;				
			} else if (cs == TEX_COLOR_SPACE_LINEAR) {
				image.format = TEX_FORMAT_RGBA8;				
			} else {
				AB_CORE_FATAL("Invalid color space.");
			}
			
			image.bit_per_pixel = bitsPerPixel;
			for (int64 i = 0; i < image.height * image.width; i++) {
				uint32 tmp;
				uint32* pixel = ((uint32*)image.bitmap) + i;
				//uint32 a = *pixel & 0xff000000;
				//uint32 r = *pixel & 0x00ff0000;
				//uint32 g = *pixel & 0x0000ff00;
				//uint32 b = *pixel & 0x000000ff;
				//*pixel = a | (r >> 16) | g | (b << 16);
				tmp = 0xff00ff00 & *pixel;
				*pixel = tmp | ((*pixel & 0x000000ff) << 16) | ((*pixel & 0x00ff0000) >> 16);
			}
		}
		else if (bitsPerPixel == 24) {
			if (cs == TEX_COLOR_SPACE_SRGB) {
				image.format = TEX_FORMAT_SRGB8;				
			} else if (cs == TEX_COLOR_SPACE_LINEAR) {
				image.format = TEX_FORMAT_RGB8;				
			} else {
				AB_CORE_FATAL("Invalid color space.");
			}
			
			image.bit_per_pixel = bitsPerPixel;
			for (int64 i = 0; i < (image.height * image.width * 3) - 3; i += 3) {
				byte* pixel = ((byte*)image.bitmap) + i;
				byte red = *(pixel);
				*pixel = *(pixel + 2);
				*(pixel + 2) = red;
			}

		}
		else {
			AB_CORE_WARN("Failed to load BMP image: %s. Wrong pixel size.", filename);
			memset(&image, 0, sizeof(Image));
			// TODO: use stack and deallocate
			//AB::DebugFreeFileMemory(data);
			return image;
		}
		return image;
	}
	// TODO: use stack and deallocate
#if 0
	void DeleteBitmap(void* ptr) {
		void* actualMemory = (void*)*((uintptr*)ptr - 1);
		AB::DebugFreeFileMemory(actualMemory);
	}
#endif

}
