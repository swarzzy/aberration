#pragma once
#include "Shared.h"

namespace AB
{
	struct Camera;
	struct ChunkMesher;
	
	const i32 TILEMAP_SAFE_MARGIN = 16 * 16;
	const i32 CHUNK_SAFE_MARGIN = 16;
	const i32 CHUNK_COORD_MIN_BOUNDARY = AB_INT32_MIN + CHUNK_SAFE_MARGIN;
	const i32 CHUNK_COORD_MAX_BOUNDARY = AB_INT32_MAX - CHUNK_SAFE_MARGIN;
	const i32 INVALID_CHUNK_COORD = AB_INT32_MAX;
	const u32 INVALID_TILE_COORD = AB_UINT32_MAX;
	const u32 CHUNK_TABLE_SIZE = 4096;

	const u32 WORLD_CHUNK_DIM_TILES = 32;
	const u32 WORLD_CHUNK_TILE_COUNT = WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES;

	const u32 MAX_LOW_ENTITIES = 10000;
	const u32 MAX_HIGH_CHUNKS = 16;
	const u32 MAX_HIGH_ENTITIES = 2048;

	const u32 ENTITY_BLOCK_CAPACITY = 16;

	const u32 ENTITY_MAX_MESHES = 4;


	struct WorldPosition
	{
		i32 chunkX;
		i32 chunkY;
		// NOTE: Offset in chunk 0..chunkSizeUnits
		v3 offset;
	};

	struct TileWorldPos
	{
		i32 chunkX;
		i32 chunkY;
		u32 tileX;
		u32 tileY;
		u32 tileZ;
	};

	struct TileCoord
	{
		u32 x, y, z;
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
		ENTITY_TYPE_WALL,
		ENTITY_TYPE_GIZMOS
	};

	struct LowEntity
	{
		u32 lowIndex;
		EntityType type;
		WorldPosition worldPos;
		f32 accelerationAmount;
		f32 size;
		//BBoxAligned aabb;
		v3 color;
		f32 friction;
		u32 highIndex;
		v2 velocity;
		u32 meshCount;
		Mesh* meshes[ENTITY_MAX_MESHES];
	};

	struct HighEntity
	{
		v3 pos; // readonly
		u32 lowIndex;
	};

	struct Entity
	{
		HighEntity* high;
		LowEntity* low;
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
		TileCoord coord;
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
		b32 dirty;
		b32 high;
		i32 coordX;
		i32 coordY;
		i32 coordZ;
		
		TerrainTileData terrainTiles[WORLD_CHUNK_TILE_COUNT];
		EntityBlock firstEntityBlock;
		Chunk* nextChunk;
	};

	//
	struct World
	{
		ChunkMesher* chunkMesher;
		f32 tileSizeInUnits;
		f32 tileRadiusInUnits;
		f32 tileSizeRaw;
		f32 toUnits;
		f32 unitsToRaw;
		f32 chunkSizeUnits;

		u32 chunkCount;

		Chunk* chunkTable[CHUNK_TABLE_SIZE];
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

	struct RaycastResult
	{
		b32 hit;
		f32 tMin;
	};

	struct TilemapRaycastResult
	{
		b32 hit;
		f32 tMin;
		TileWorldPos pos;
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

	// NOTE: These should be static and SHOULDN'T BE CALLED
	// FROM EXTERNAL CODE EVER
	u32
		_SetEntityToHigh(World* world, WorldPosition camTargetWorldPos,
						 u32 lowIndex);

	void
		_SetEntityToLow(World* world, u32 highIndex);

	inline Chunk*
		GetChunk(World* world, i32 chunkX, i32 chunkY,
				 MemoryArena* arena = nullptr);

	inline TerrainTileData*
		GetTerrainTileInternal(Chunk* chunk, u32 tileInChunkX,
							   u32 tileInChunkY, u32 tileInChunkZ);

	inline TerrainTileData
		GetTerrainTile(Chunk* chunk, u32 tileInChunkX,
					   u32 tileInChunkY, u32 tileInChunkZ);

	void
		SetTerrainTile(Chunk* chunk, u32 tileX, u32 tileY, u32 tileZ,
					   TerrainTileData* data); 

	TerrainTile
		FindTileBeforeFirstGapInCell(World* world, Chunk* chunk,
									 TileCoord beginCoord);

	inline TileWorldPos
		InvalidTileWorldPos();

	inline bool
		IsValid(TileWorldPos pos);

	inline TileCoord
		ChunkRelOffsetToTileCoord(World* world, v3 chunkRelOffset);
	
	inline WorldPosition
		OffsetWorldPos(World* world, WorldPosition oldPos, v3 offset);

	inline bool
		IsNormalized(World* world, WorldPosition* pos);

	inline WorldPosition
		NormalizeWorldPos(World* world, WorldPosition* pos);

	void
		ChangeEntityPos(World* world, LowEntity* entity,
						WorldPosition newPos, WorldPosition camTragetWorldPos,
						MemoryArena* arena);

	inline void
		OffsetEntityPos(World* world, LowEntity* entity,
						v3 offset, WorldPosition camTargetWorldPos,
						MemoryArena* arena);

	inline v3
		WorldPosDiff(World* world, WorldPosition a, WorldPosition b);

	u32
		AddLowEntity(World* world, Chunk* chunk, EntityType type,
					 MemoryArena* arena = nullptr);

	inline v3 // NOTE: z is still relative to global sea level
		GetCamRelPos(World* world, WorldPosition worldPos,
					 WorldPosition camTargetWorldPos);

	inline v3
		FlipYZ(v3 worldCoord);

	u32 // NOTE: LowEntityIndex
		Raycast(World* world, Camera* camera, v3 from, v3 dir);

	RaycastResult
		RayAABBIntersection(v3 rayFrom, v3 rayDir, v3 boxMin, v3 boxMax);

	TilemapRaycastResult
		TilemapRaycast(World* world, Camera* camera, v3 from, v3 dir);

	void
		SetChunkToLow(World* world, Chunk* chunk);

	void
		SetChunkToHigh(World* world, Chunk* chunk,
					   WorldPosition camTargetWorldPos);

	u32
		AddWallEntity(World* world, Chunk* chunk, v2 offset,
					  AssetManager* assetManager, MemoryArena* arena = 0);

	inline bool NotEmpty(TerrainTileData* tile)
	{
		return tile && tile->type;
	}

}
