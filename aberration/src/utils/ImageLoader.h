#pragma once
#include "src/ABHeader.h"

namespace AB {

	enum class PixelFormat : uint32 {
		RGB = 0,
		RGBA,
		RED
	};

	struct Image {
		uint32 width;
		uint32 height;
		PixelFormat format;
		byte* bitmap;
	};

	Image LoadBMP(const char* filename);
	void DeleteBitmap(void* ptr);

#pragma pack(push, 1)
	// Size: 12 bytes
	struct BMPHeader {
		uint16 type;
		uint32 size;
		uint16 reserved1;
		uint16 reserved2;
		uint32 offsetToBitmap;
	};

	// Size: 40 bytes
	struct BMPInfoHeaderCore {
		uint32 structSize;
		uint16 width;
		uint16 height;
		uint16 planes;
		uint16 bitsPerPixel;
		//uint32 compression;
	};

	struct BMPInfoHeaderV3Cut {
		uint32 structSize;
		int32 width;
		int32 height;
		uint16 planes;
		uint16 bitsPerPixel;
	};

	// Size: 108 bytes
	struct BMPInfoHeaderV3 {
		uint32 structSize;
		int32 width;
		int32 height;
		uint16 planes;
		uint16 bitsPerPixel;
		uint32 compression;
		uint32 imageSize;
		int32 pixelsPerMeterX;
		int32 pixelsPerMeterY;
		uint32 colorIndUsed;
		uint32 colorIndRequired;
	};

	struct BMPInfoHeaderV4 {
		uint32 structSize;
		int32 width;
		int32 height;
		uint16 planes;
		uint16 bitsPerPixel;
		uint32 compression;
		uint32 imageSize;
		int32 pixelsPerMeterX;
		int32 pixelsPerMeterY;
		uint32 colorIndUsed;
		uint32 colorIndRequired;
		uint32 redMask;
		uint32 greenMask;
		uint32 blueMask;
		uint32 alphaMask;
		uint32 colorSpaceType;
		int32 CIE[9];
		uint32 redGamma;
		uint32 greenGamma;
		uint32 blueGamma;
	};
#pragma pack(pop)
}
