#pragma once
#include "Shared.h"

namespace AB
{
	struct Camera;
	
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
		// NOTE: Height in units relative to sea level (which is 0.0)
		f32 z;
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
		v3 size;
		//BBoxAligned aabb;
		//Mesh* mesh;
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

	World*
	CreateWorld(MemoryArena* arena);

	inline LowEntity*
	GetLowEntity(World* world, u32 lowIndex);

	inline HighEntity*
	GetHighEntity(World* world, u32 highIndex);

	inline Entity
	GetEntityFromLowIndex(World* world, u32 lowIndex);

	inline Entity
	GetEntityFromHighIndex(World* world, u32 highIndex);

	u32
	_SetEntityToHigh(World* world, u32 lowIndex);

	void
	_SetEntityToLow(World* world, u32 highIndex);

	inline Chunk*
	GetChunk(World* world, i32 chunkX, i32 chunkY, MemoryArena* arena = nullptr);

	inline TerrainType
	GetTerrainTile(Chunk* chunk, u32 tileInChunkX, u32 tileInChunkY);

	inline void
	SetTerrainTile(Chunk* chunk, u32 tileX, u32 tileY, TerrainType type);

	inline WorldPosition
	OffsetWorldPos(World* world, WorldPosition oldPos, v3 offset);

	void
	ChangeEntityPos(World* world, LowEntity* entity,
					WorldPosition newPos, MemoryArena* arena);

	inline v3
	WorldPosDiff(World* world, WorldPosition a, WorldPosition b);

	u32
	AddLowEntity(World* world, Chunk* chunk, EntityType type,
				 MemoryArena* arena = nullptr);

	inline v3 // NOTE: z is still relative to global sea level
	GetCamRelPos(World* world, WorldPosition worldPos,
				 WorldPosition camTargetWorldPos);

	inline v3
	ConvertWorldToRendererCoord(v3 worldCoord);

	u32 // NOTE: LowEntityIndex
	Raycast(World* world, Camera* camera, v3 from, v3 dir);

	u32
	AddWallEntity(World* world, Chunk* chunk, v2 offset, f32 z,
				  MemoryArena* arena = 0);
}
