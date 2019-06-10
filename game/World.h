#pragma once
#include "Shared.h"

namespace AB
{
	const i32 TILEMAP_SAFE_MARGIN = 16 * 16;
	const i32 CHUNK_SAFE_MARGIN = 16;
	const u32 INVALID_CHUNK_COORD = AB_INT32_MAX;
	const u32 CHUNK_TABLE_SIZE = 4096;

	const u32 WORLD_CHUNK_DIM_TILES = 64;

	const u32 MAX_LOW_ENTITIES = 10000;
	const u32 MAX_HIGH_CHUNKS = 16;
	const u32 MAX_HIGH_ENTITIES = 2048;

	struct WorldPosition
	{
		i32 chunkX;
		i32 chunkY;
		// NOTE: Offset in chunk 0..chunkSizeUnits
		v2 offset;
	};

	enum EntityResidence
	{
		ENTITY_RESIDENCE_NOT_EXIST = 0,
		ENTITY_RESIDENCE_HIGH,
		ENTITY_RESIDENCE_LOW
	};

	enum EntityType
	{
		ENTITY_TYPE_BODY,
		ENTITY_TYPE_WALL
	};
	
	struct LowEntity
	{
		u32 lowIndex;
		EntityType type;
		WorldPosition worldPos;
		f32 accelerationAmount;
		v2 size;
		v3 color;
		f32 friction;
		u32 highIndex;
		v2 velocity;
	};

	struct HighEntity
	{
		//v2 pos;
		u32 lowIndex;
	};

	struct Entity
	{
		HighEntity* high;
		LowEntity* low;
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

	// TODO: High chunk list
	struct Chunk
	{
		b32 high;
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
		f32 chunkSizeUnits;

		u32 chunkCount;

		Chunk chunkTable[CHUNK_TABLE_SIZE];
		u32 nonResidentEntityBlocksCount;
		u32 freeEntityBlockCount;
		EntityBlock* firstFreeBlock;
		u32 highChunkCount;
		Chunk* highChunks[MAX_HIGH_CHUNKS];

		u32 lowEntityCount;
		u32 highEntityCount;
		HighEntity highEntities[MAX_HIGH_ENTITIES];
		LowEntity lowEntities[MAX_LOW_ENTITIES];

	};
}
