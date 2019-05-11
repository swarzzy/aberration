#pragma once
#include "AB.h"

namespace AB
{
	
	struct TilemapPosition
	{
		u32 tileX;
		u32 tileY;
		// NOTE: Realtive to tile
		v2 offset;
	};

	struct ChunkPosition
	{
		u32 chunkX;
		u32 chunkY;
		u32 tileInChunkX;
		u32 tileInChunkY;
	};

	struct Chunk
	{
		u32* tiles;
		v3* colors;
	};

	struct Tilemap
	{
		f32 tileSizeInUnits;
		f32 tileRadiusInUnits;
		f32 tileSizeRaw;
		f32 toUnits;
		f32 unitsToRaw;

		u32 chunkShift;
		u32 chunkMask;

		u32 chunkSizeInTiles;
		u32 tilemapChunkCountX;
		u32 tilemapChunkCountY;
		
		Chunk* chunks;
	};
}
