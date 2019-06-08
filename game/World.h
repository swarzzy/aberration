#pragma once
#include "Shared.h"

namespace AB
{
	const i32 TILEMAP_SAFE_MARGIN = 16 * 16;
	// TODO: @Important !!!! Safe margin and stuff
	const u32 INVALID_CHUNK_COORD = AB_INT32_MAX;
	const u32 CHUNK_TABLE_SIZE = 4096;

	const i32 WORLD_CHUNK_SHIFT = 6;
	// TODO: Do we need that now
	const u32 WORLD_CHUNK_DIM_TILES = (u32)(1 << WORLD_CHUNK_SHIFT);
	const i32 WORLD_CHUNK_MASK = (1 << WORLD_CHUNK_SHIFT) - 1;

	struct WorldPosition
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

	enum TerrainType
	{
		TERRAIN_TYPE_EMPTY = 0,
		TERRAIN_TYPE_GRASS,
		TERRAIN_TYPE_CLIFF,
		TERRAIN_TYPE_WATER
	};

	const u32 ENTITY_BLOCK_CAPACITY = 16;

	struct EntityBlock
	{
		u32 count;
		u32 lowEntityIndices[ENTITY_BLOCK_CAPACITY];
		EntityBlock* nextBlock;
	};
	
	struct Chunk
	{
		i32 coordX;
		i32 coordY;
		
		TerrainType terrainTiles[WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES];
		EntityBlock firstEntityBlock;
		Chunk* nextChunk;
	};

	struct World
	{
		f32 tileSizeInUnits;
		f32 tileRadiusInUnits;
		f32 tileSizeRaw;
		f32 toUnits;
		f32 unitsToRaw;

		u32 chunkCountX;
		u32 chunkCountY;
		
		Chunk chunkTable[CHUNK_TABLE_SIZE];
	};
}
