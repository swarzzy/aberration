#include "ImageLoader.h"
#include "src/utils/Log.h"
#include "src/platform/Platform.h"

namespace AB {
	
	Image LoadBMP(const char* filename) {
		Image image = {};
		uint32 dataSize;
		byte* data = (byte*)AB::DebugReadFile(filename, &dataSize);
		if (!data) {
			AB_CORE_WARN("Failed to load BMP image: ", filename, ". Failed to open file.");
			return image;
		}

		BMPHeader* header = (BMPHeader*)data;
		BMPInfoHeaderCore* infoHeader = (BMPInfoHeaderCore*)(data + sizeof(BMPHeader));
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
						image.height = (uint32)std::abs(v3->height);
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
							image.height = (uint32)std::abs(v4->height);
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
							if (((uint32)(std::abs(v3c->height) * v3c->width)) == (header->size / v3c->bitsPerPixel)) {
								if (v3c->height < 0) {
									bottomUp = false;
									image.height = (uint32)std::abs(v3c->height);
								}
								else {
									image.height = v3c->height;
								}
								image.width = v3c->width;
								bitsPerPixel = v3c->bitsPerPixel;
							}
							AB_CORE_WARN("Failed to load BMP image: ", filename, ". Unknown format version.");
							memset(&image, 0, sizeof(Image));
							AB::DebugFreeFileMemory(data);
							return image;

						}
						else {
							AB_CORE_WARN("Failed to load BMP image: ", filename, ". Unknown format version.");
							memset(&image, 0, sizeof(Image));
							AB::DebugFreeFileMemory(data);
							return image;
						}
					} break;
		}

		// TODO: TEMPORARY: Abort loading if pixel order is top->down 
		if (!bottomUp) {
			AB_CORE_WARN("Failed to load BMP image: ", filename, ". Wrong data order.");
			memset(&image, 0, sizeof(Image));
			AB::DebugFreeFileMemory(data);
			return image;
		}

		image.bitmap = (byte*)data + header->offsetToBitmap;
		// TODO: TEMPORARY: Store pointer to a beginning of block right before bitmap
		*((uintptr*)image.bitmap - 1) = (uintptr)data;

		if (bitsPerPixel == 32) {
			image.format = PixelFormat::RGBA;
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
			image.format = PixelFormat::RGB;
			for (int64 i = 0; i < (image.height * image.width * 3) - 3; i += 3) {
				byte* pixel = ((byte*)image.bitmap) + i;
				byte red = *(pixel);
				*pixel = *(pixel + 2);
				*(pixel + 2) = red;
			}

		}
		else {
			AB_CORE_WARN("Failed to load BMP image: ", filename, ". Wrong pixel size.");
			memset(&image, 0, sizeof(Image));
			AB::DebugFreeFileMemory(data);
			return image;
		}
		return image;
	}

	void DeleteBitmap(void* ptr) {
		void* actualMemory = (void*)*((uintptr*)ptr - 1);
		AB::DebugFreeFileMemory(actualMemory);
	}

}