#pragma once
#include "Shared.h"

namespace AB
{	
	struct TilemapPosition
	{
		i32 tileX;
		i32 tileY;
		// NOTE: Realtive to tile
		v2 offset;
	};

	struct ChunkPosition
	{
		i32 chunkX;
		i32 chunkY;
		u32 tileInChunkX;
		u32 tileInChunkY;
	};

	struct Chunk
	{
		v2u coord;
		u32* tiles;
		v3* colors;
		Chunk* nextChunk;
	};

	const i32 TILEMAP_SAFE_MARGIN = 16 * 16;
	// TODO: @Important !!!! Safe margin and stuff
	const u32 INVALID_CHUNK_COORD = AB_INT32_MAX;
	const u32 CHUNK_TABLE_SIZE = 4096;

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
		u32 chunkCountX;
		u32 chunkCountY;
		
		Chunk chunkTable[CHUNK_TABLE_SIZE];
	};
}
