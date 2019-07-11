#pragma once
#include "Shared.h"

namespace AB
{
	struct Camera;
	struct ChunkMesher;
	
	const i32 CHUNK_SAFE_MARGIN = 16;
	const i32 CHUNK_COORD_MIN_BOUNDARY = AB_INT32_MIN + CHUNK_SAFE_MARGIN;
	const i32 CHUNK_COORD_MAX_BOUNDARY = AB_INT32_MAX - CHUNK_SAFE_MARGIN;
	const i32 INVALID_CHUNK_COORD = AB_INT32_MAX;
	const i32 INVALID_TILE_COORD = AB_INT32_MAX;
	const u32 CHUNK_TABLE_SIZE = 4096;

	const i32 WORLD_CHUNK_SHIFT = 5;
	const u32 WORLD_CHUNK_MASK = (u32)(1 << WORLD_CHUNK_SHIFT) - 1;

	const u32 WORLD_CHUNK_DIM_TILES = (u32)(1 << WORLD_CHUNK_SHIFT);
	const i32 TILEMAP_SAFE_MARGIN = 16 * WORLD_CHUNK_DIM_TILES;
	const i32 TILE_COORD_MIN_BOUNDARY = AB_INT32_MIN + TILEMAP_SAFE_MARGIN;
	const i32 TILE_COORD_MAX_BOUNDARY = AB_INT32_MAX - TILEMAP_SAFE_MARGIN;
	
	const u32 WORLD_CHUNK_TILE_COUNT = WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES;

	const u32 MAX_LOW_ENTITIES = 10000;

	const u32 ENTITY_BLOCK_CAPACITY = 16;

	const f32 WORLD_TILE_SIZE = 1.0f;
	const f32 WORLD_TILE_RADIUS = WORLD_TILE_SIZE * 0.5f;
	const f32 WORLD_CHUNK_SIZE = WORLD_TILE_SIZE * WORLD_CHUNK_DIM_TILES;

	const u32 ENTITY_MAX_MESHES = 4;

	enum EntityType
	{
		ENTITY_TYPE_BODY,
		ENTITY_TYPE_WALL,
		ENTITY_TYPE_GIZMOS
	};

	enum SimulationType : u32
	{
		SIM_TYPE_INACTIVE = 0,
		SIM_TYPE_ACTIVE,
		SIM_TYPE_DORMANT
	};

    struct Entity
    {
		u32 id;
		SimulationType simType;
        v3 pos;
		EntityType type;
		f32 accelerationAmount;
		f32 distanceLimit;
		f32 size;
		v3 color;
		f32 friction;
		u32 highIndex;
		v3 velocity;
		u32 meshCount;
		Mesh* meshes[ENTITY_MAX_MESHES];
    };

	struct WorldPosition
	{
		v3i tile;
		v3 offset;
	};

	struct ChunkPosition
	{
		v3i chunk;
		v3u tile;
	};

	struct StoredEntity
	{
		Entity storage;
		WorldPosition worldPos;
	};

	enum TerrainType : u32
	{
		TERRAIN_TYPE_EMPTY = 0,
		TERRAIN_TYPE_GRASS,
		TERRAIN_TYPE_CLIFF,
		TERRAIN_TYPE_WATER
	};

	struct TerrainTileData
	{
	   	TerrainType type;
	};

	struct TerrainTile
	{
		TerrainTileData data;
		v3i coord;
	};
	
	struct EntityBlock
	{
		u32 count;
		u32 lowEntityIndices[ENTITY_BLOCK_CAPACITY];
		EntityBlock* nextBlock;
	};

	// TODO: High chunk list
	struct Chunk
	{
		// TODO: This flags here are defintly not a good idea
		b32 dirty;
		SimulationType simType;
		b32 visible;

		// TODO: v3i
		i32 coordX;
		i32 coordY;
		i32 coordZ;
		
		TerrainTileData terrainTiles[WORLD_CHUNK_TILE_COUNT];
		u32 entityCount;
		EntityBlock firstEntityBlock;
		Chunk* nextChunk;
	};

	//
	struct World
	{
		ChunkMesher* chunkMesher;

		u32 chunkCount;
		Chunk* chunkTable[CHUNK_TABLE_SIZE];
		u32 nonResidentEntityBlocksCount;
		u32 freeEntityBlockCount;
		EntityBlock* firstFreeBlock;

		u32 lowEntityCount;
		StoredEntity lowEntities[MAX_LOW_ENTITIES];

	};

	struct RaycastResult
	{
		b32 hit;
		f32 tMin;
		v3 normal;
	};

	struct TilemapRaycastResult
	{
		b32 hit;
		f32 tMin;
		ChunkPosition coord;
		v3 normal;
	};

	struct ChunkRegion
	{
		v3i minBound;
		v3i maxBound;
	};

	struct TileRegion
	{
		v3i minBound;
		v3i maxBound;		
	};
}
