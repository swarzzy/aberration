#include "World.h"
#include "render/ChunkMesher.h"
namespace AB
{
	World*
	CreateWorld(MemoryArena* arena);

	inline StoredEntity*
	GetStoredEntity(World* world, u32 lowIndex);

	inline Chunk*
	GetChunk(World* world, v3i chunk, MemoryArena* arena = 0);

	static inline EntityBlock*
	GetEmptyEntityBlock(World* world, MemoryArena* arena = nullptr);

	u32
	AddStoredEntity(World* world, Chunk* chunk, EntityType type, MemoryArena* arena);

	u32
	AddWallEntity(World* world, Chunk* chunk, v3 offset,
				  AssetManager* assetManager, MemoryArena* arena);

	inline TerrainTileData*
	GetTerrainTileInternal(Chunk* chunk, u32 tileInChunkX, u32 tileInChunkY,
						   u32 tileInChunkZ);

	inline TerrainTileData
	GetTerrainTile(Chunk* chunk, v3u tileInChunk);

	void
	SetTerrainTile(Chunk* chunk, v3u tileInChunk, TerrainTileData* data);

	inline bool
	NotEmpty(TerrainTileData* tile);

	inline i32
	SafeAddTileCoord(i32 a, i32 b);

	inline i32
	SafeSubTileCoord(i32 a, i32 b);

	inline i32
	_SafeAddChunkCoord(i32 a, i32 b);	

	inline i32
	_SafeAddChunkCoord(i32 a, i32 b);

	inline ChunkRegion
	ChunkRegionFromOriginAndSpan(v3i origin, v3i span);

	inline bool
	InChunkRegion(ChunkRegion* region, v3i coord);

	inline WorldPosition
	OffsetWorldPos(WorldPosition oldPos, v3 offset);

	inline bool
	IsNormalized(WorldPosition* pos);

	inline WorldPosition
	NormalizeWorldPos(WorldPosition* pos);

	inline ChunkPosition
	InvalidChunkPos();

	inline WorldPosition
	InvalidWorldPos();

	inline bool
	IsValid(WorldPosition* pos);

	inline bool
	IsValid(ChunkPosition* pos);

	inline WorldPosition
	GetWorldPos(ChunkPosition* pos);

	inline ChunkPosition
	GetChunkPos(WorldPosition* pos);

	WorldPosition
	ChangeEntityPos(World* world, StoredEntity* entity,	WorldPosition newPos,
					WorldPosition camTargetWorldPos, MemoryArena* arena);

	inline WorldPosition
	OffsetEntityPos(World* world, StoredEntity* entity,	v3 offset,
					WorldPosition camTargetWorldPos, MemoryArena* arena);

	inline v3
	WorldPosDiff(WorldPosition* a, WorldPosition* b);

	inline v3
	GetRelativePos(WorldPosition* origin, WorldPosition* target);

	// TODO: Make that world and renderer have same coords
	inline v3
	FlipYZ(v3 worldCoord);

	RaycastResult
	RayAABBIntersection(v3 rayFrom, v3 rayDir, v3 boxMin, v3 boxMax);

	static u32 // NOTE: Entity ID
	RaycastEntities(SimRegion* region, v3 from, v3 dir);

	TilemapRaycastResult
	TilemapRaycast(SimRegion* region, v3 from, v3 dir);
	
	World*
	CreateWorld(MemoryArena* arena)
	{
		World* world = nullptr;
		world = (World*)PushSize(arena, sizeof(World), alignof(World));
		//SetZeroScalar(World, world);

		world->chunkMesher = (ChunkMesher*)PushSize(arena, sizeof(ChunkMesher), 0);
		AB_ASSERT(world->chunkMesher);

		InitializeMesher(world->chunkMesher);
		
		// NOTE: Reserving for null entity
		world->lowEntityCount = 1;
		return world;
	}

	inline StoredEntity*
	GetStoredEntity(World* world, u32 lowIndex)
	{
		StoredEntity* result = nullptr;
		if (lowIndex > 0 && lowIndex < world->lowEntityCount)
		{
			result = world->lowEntities + lowIndex;
		}
		return result;		
	}

	inline Chunk*
	GetChunk(World* world, v3i chunk, MemoryArena* arena)
	{
		Chunk* result = nullptr;
		// TODO: Check is chunk coord are valid
		// TODO: Better hash function !!111
		u32 chunkHash = (AbsI32(chunk.x) * 22 + AbsI32(chunk.y) * 12 + 7 * AbsI32(chunk.z))
			% CHUNK_TABLE_SIZE;
		AB_ASSERT(chunkHash < CHUNK_TABLE_SIZE);

		Chunk* currentChunk = world->chunkTable[chunkHash];
		if (currentChunk)
		{
			if ((currentChunk->coordX == chunk.x) &&
				(currentChunk->coordY == chunk.y) &&
				(currentChunk->coordZ == chunk.z))
			{
				result = currentChunk;
			}
			else
			{
				while (currentChunk->nextChunk)
				{
					Chunk* nextChunk = currentChunk->nextChunk;
					if ((nextChunk->coordX == chunk.x) &&
						(nextChunk->coordY == chunk.y) &&
						(nextChunk->coordZ == chunk.z))
					{
						result = nextChunk;
						break;
					}
					else
					{
						currentChunk = currentChunk->nextChunk;
					}
				}
			}
		}

		if (!result && arena)
		{
			Chunk* newChunk;
			
			newChunk = (Chunk*)PushSize(arena, sizeof(Chunk),
										alignof(Chunk));
			AB_ASSERT(newChunk);

			if (!currentChunk)
			{
				world->chunkTable[chunkHash] = newChunk;
			}
			else
			{
				currentChunk->nextChunk = newChunk;
			}
			
			newChunk->coordX = chunk.x;
			newChunk->coordY = chunk.y;
			newChunk->coordZ = chunk.z;
			newChunk->nextChunk = nullptr;
			world->chunkCount++;

			result = newChunk;
		}
		
		return result;
	}

	static inline EntityBlock*
	GetEmptyEntityBlock(World* world, MemoryArena* arena)
	{
		EntityBlock* block = nullptr;
		if (world->firstFreeBlock)
		{
			block = world->firstFreeBlock;
			world->firstFreeBlock = block->nextBlock;
			world->freeEntityBlockCount--;
			SetZeroScalar(EntityBlock, block);
		}
		else if (arena)
		{
			block =	(EntityBlock*)PushSize(arena, sizeof(EntityBlock),
										   alignof(EntityBlock));
			world->nonResidentEntityBlocksCount++;
			SetZeroScalar(EntityBlock, block);
		}
		return block;
	}

	u32
	AddStoredEntity(World* world, Chunk* chunk, EntityType type,
					MemoryArena* arena)
	{
		AB_ASSERT(world->lowEntityCount < MAX_LOW_ENTITIES);
		u32 index = 0;
		if (chunk->firstEntityBlock.count < ENTITY_BLOCK_CAPACITY)
		{
			index = world->lowEntityCount;
			world->lowEntities[index] = {};
			world->lowEntities[index].storage.type = type;
			world->lowEntities[index].storage.id = index;
			WorldPosition pos = {chunk->coordX, chunk->coordY,chunk->coordZ, V3(0.0f)};
			world->lowEntities[index].worldPos = pos;
		
			chunk->firstEntityBlock.lowEntityIndices[chunk->firstEntityBlock.count]
				= index;
			chunk->firstEntityBlock.count++;
			world->lowEntityCount++;
			chunk->entityCount++;
		}
		else
		{
			EntityBlock* emptyBlock = GetEmptyEntityBlock(world, arena);
			if (emptyBlock)
			{
				*emptyBlock = chunk->firstEntityBlock;
				chunk->firstEntityBlock.count = 0;
				chunk->firstEntityBlock.nextBlock = emptyBlock;
				chunk->firstEntityBlock.count++;

				index = world->lowEntityCount;
				world->lowEntities[index] = {};
				world->lowEntities[index].storage.type = type;
				world->lowEntities[index].storage.id = index;
		
				chunk->firstEntityBlock.lowEntityIndices[0] = index;
				world->lowEntityCount++;
				chunk->entityCount++;
			}
		}

		return index;
	}
	
	// TODO: Think about what's happens if offset is out if chunk bounds
	u32
	AddWallEntity(World* world, Chunk* chunk, v3 offset,
				  AssetManager* assetManager, MemoryArena* arena)
	{
		// assert for coord out of chunk bounds
		u32 index = 0;
		index = AddStoredEntity(world, chunk, ENTITY_TYPE_WALL, arena);
		if (index)
		{
			Entity* stored = &world->lowEntities[index].storage;
			stored->type = ENTITY_TYPE_WALL;
			stored->meshCount = 1;
			stored->meshes[0] = assetManager->meshes;
		}
		
		return index;
	}
	
	inline TerrainTileData*
	GetTerrainTileInternal(Chunk* chunk, u32 tileInChunkX, u32 tileInChunkY,
						   u32 tileInChunkZ)
	{
		TerrainTileData* result = nullptr;
#if 0
		AB_ASSERT(tileInChunkX < WORLD_CHUNK_DIM_TILES);
		AB_ASSERT(tileInChunkY < WORLD_CHUNK_DIM_TILES);
		AB_ASSERT(tileInChunkZ < WORLD_CHUNK_DIM_TILES);
#endif
		
		if (chunk &&
			(tileInChunkX < WORLD_CHUNK_DIM_TILES) &&
			(tileInChunkY < WORLD_CHUNK_DIM_TILES) &&
			(tileInChunkZ < WORLD_CHUNK_DIM_TILES))
		{
			u32 offset =
				tileInChunkY * WORLD_CHUNK_DIM_TILES * WORLD_CHUNK_DIM_TILES +
				tileInChunkX * WORLD_CHUNK_DIM_TILES +
				tileInChunkZ;
			result = chunk->terrainTiles + offset;
		}
		
		return result;
	}
	
	inline TerrainTileData
	GetTerrainTile(Chunk* chunk, v3u tileInChunk)
	{
		TerrainTileData result = {};
		TerrainTileData* ptr = GetTerrainTileInternal(chunk, tileInChunk.x,
													  tileInChunk.y, tileInChunk.z);
		if (ptr)
		{
			result = *ptr;
		}
		
		return result;
	}

	void
	SetTerrainTile(Chunk* chunk, v3u tileInChunk,
				   TerrainTileData* data)
	{
		TerrainTileData* tile = GetTerrainTileInternal(chunk, tileInChunk.x,
													   tileInChunk.y, tileInChunk.z);
		*tile = *data;
		chunk->dirty = true;
	}

	inline bool
	NotEmpty(TerrainTileData* tile)
	{
		bool result = false;
		if (tile)
		{
			result = (bool)tile->type;			
		}
		return result;
	}


	inline i32
	SafeSubTileCoord(i32 a, i32 b)
	{
		i32 result = 0;
		if (b < 0)
		{
			result = SafeAddTileCoord(a, -b);
		}
		else
		{
			result = a - b;
			if (result > a)
			{
				result = AB_INT32_MIN + TILEMAP_SAFE_MARGIN + 1;
			}
		}
		
		return result;
	}

	inline i32
	SafeAddTileCoord(i32 a, i32 b)
	{
		i32 result;
		if (b < 0)
		{
			result = SafeSubTileCoord(a, -b);
		}
		else
		{
			result = a + b;
			if (result < a)
			{
				result = AB_INT32_MAX - TILEMAP_SAFE_MARGIN - 1;
			}
		}

		return result;
	}

	inline i32
	_SafeSubChunkCoord(i32 a, i32 b)
	{
		i32 result = 0;
		if (b < 0)
		{
			result = _SafeAddChunkCoord(a, -b);
		}
		else
		{
			result = a - b;
			if (result > a)
			{
				result = AB_INT32_MIN + CHUNK_SAFE_MARGIN + 1;
			}
		}
		
		return result;
	}

	inline i32
	_SafeAddChunkCoord(i32 a, i32 b)
	{
		i32 result;
		if (b < 0)
		{
			result = _SafeSubChunkCoord(a, -b);
		}
		else
		{
			result = a + b;
			if (result < a)
			{
				result = AB_INT32_MAX - CHUNK_SAFE_MARGIN - 1;
			}
		}

		return result;
	}

	inline ChunkRegion ChunkRegionFromOriginAndSpan(v3i origin, v3i span)
	{
		ChunkRegion result;

		result.minBound.x = _SafeSubChunkCoord(origin.x, span.x);
		result.minBound.y = _SafeSubChunkCoord(origin.y, span.y);
		result.minBound.z = _SafeSubChunkCoord(origin.z, span.z);
		result.maxBound.x = _SafeAddChunkCoord(origin.x, span.x);
		result.maxBound.y = _SafeAddChunkCoord(origin.y, span.y);
		result.maxBound.z = _SafeAddChunkCoord(origin.z, span.z);

		return result;
	}

	inline bool InChunkRegion(ChunkRegion* region, v3i coord)
	{
		bool result = coord >= region->minBound &&	coord <= region->maxBound;
		return result;			
	}
	
	inline WorldPosition
	OffsetWorldPos(WorldPosition oldPos, v3 offset)
	{
		// TODO: Better checking!
		WorldPosition newPos = oldPos;
		newPos.offset += offset;
		i32 tileOffsetX = Floor(newPos.offset.x / WORLD_TILE_SIZE);
		i32 tileOffsetY = Floor(newPos.offset.y / WORLD_TILE_SIZE);
		i32 tileOffsetZ = Floor(newPos.offset.z / WORLD_TILE_SIZE);

		newPos.offset.x -= tileOffsetX * WORLD_TILE_SIZE;
		newPos.offset.y -= tileOffsetY * WORLD_TILE_SIZE;
		newPos.offset.z -= tileOffsetZ * WORLD_TILE_SIZE;

		AB_ASSERT(oldPos.tile.x + tileOffsetX > INT32_MIN + TILEMAP_SAFE_MARGIN);
		AB_ASSERT(oldPos.tile.x + tileOffsetX < INT32_MAX - TILEMAP_SAFE_MARGIN);
		
		AB_ASSERT(oldPos.tile.y + tileOffsetY > INT32_MIN + TILEMAP_SAFE_MARGIN);
		AB_ASSERT(oldPos.tile.y + tileOffsetY < INT32_MAX - TILEMAP_SAFE_MARGIN);

		AB_ASSERT(oldPos.tile.z + tileOffsetZ > INT32_MIN + TILEMAP_SAFE_MARGIN);
		AB_ASSERT(oldPos.tile.z + tileOffsetZ < INT32_MAX - TILEMAP_SAFE_MARGIN);

		// TODO: Should these by safe adds?
		newPos.tile.x = SafeAddTileCoord(newPos.tile.x, tileOffsetX);
		newPos.tile.y = SafeAddTileCoord(newPos.tile.y, tileOffsetY);
		newPos.tile.z = SafeAddTileCoord(newPos.tile.z, tileOffsetZ);

		return newPos;
	}

	inline bool
	IsNormalized(WorldPosition* pos)
	{
		// NOTE: <= ???
		return ((pos->offset.x <= WORLD_TILE_SIZE) &&
				(pos->offset.y <= WORLD_TILE_SIZE) &&
				(pos->offset.z <= WORLD_TILE_SIZE));
	}

	inline WorldPosition
	NormalizeWorldPos(WorldPosition* pos)
	{
		return OffsetWorldPos(*pos, V3(0.0f));
	}

	inline ChunkPosition
	InvalidChunkPos()
	{
		ChunkPosition result;
		result.chunk = V3I(INVALID_CHUNK_COORD);
		result.tile = V3U(AB_UINT32_MAX);
		return result;
	}

	inline WorldPosition
	InvalidWorldPos()
	{
		WorldPosition result;
		result.tile = V3I(INVALID_TILE_COORD);
		result.offset = V3(AB_FLOAT_MAX);
		return result;
	}

	inline bool
	IsValid(WorldPosition* pos)
	{
		bool result = false;
		result =
			(pos->tile.x > TILE_COORD_MIN_BOUNDARY) &&
			(pos->tile.x < TILE_COORD_MAX_BOUNDARY);
		result =
			(pos->tile.y > TILE_COORD_MIN_BOUNDARY) &&
			(pos->tile.y < TILE_COORD_MAX_BOUNDARY);
		result =
			(pos->tile.z > TILE_COORD_MIN_BOUNDARY) &&
			(pos->tile.z < TILE_COORD_MAX_BOUNDARY);

		// NOTE: <=???
		result = pos->offset.x <= WORLD_TILE_SIZE;
		result = pos->offset.y <= WORLD_TILE_SIZE;
		result = pos->offset.z <= WORLD_TILE_SIZE;

		return result;
	}

	inline bool
	IsValid(ChunkPosition* pos)
	{
		bool result = false;
		result =
			(pos->chunk.x > CHUNK_COORD_MIN_BOUNDARY) &&
			(pos->chunk.x < CHUNK_COORD_MAX_BOUNDARY);
		result =
			(pos->chunk.y > CHUNK_COORD_MIN_BOUNDARY) &&
			(pos->chunk.y < CHUNK_COORD_MAX_BOUNDARY);
		result =
			(pos->chunk.z > CHUNK_COORD_MIN_BOUNDARY) &&
			(pos->chunk.z < CHUNK_COORD_MAX_BOUNDARY);

		result = pos->tile.x < WORLD_CHUNK_DIM_TILES;
		result = pos->tile.y < WORLD_CHUNK_DIM_TILES;
		result = pos->tile.z < WORLD_CHUNK_DIM_TILES;
		
		return result;
	}
		
	inline WorldPosition
	GetWorldPos(ChunkPosition* pos)
	{
		WorldPosition result = {};
		result.tile.x = pos->chunk.x << WORLD_CHUNK_SHIFT;
		result.tile.y = pos->chunk.y << WORLD_CHUNK_SHIFT;
		result.tile.z = pos->chunk.z << WORLD_CHUNK_SHIFT;

		result.tile.x += pos->tile.x;
		result.tile.y += pos->tile.y;
		result.tile.z += pos->tile.z;
		return result;
	}

	inline ChunkPosition
	GetChunkPos(WorldPosition* pos)
	{
		AB_ASSERT(IsNormalized(pos));
		ChunkPosition result;
		result.chunk.x = pos->tile.x >> WORLD_CHUNK_SHIFT;
		result.chunk.y = pos->tile.y >> WORLD_CHUNK_SHIFT;
		result.chunk.z = pos->tile.z >> WORLD_CHUNK_SHIFT;

		result.tile.x = pos->tile.x & WORLD_CHUNK_MASK;
		result.tile.y = pos->tile.y & WORLD_CHUNK_MASK;
		result.tile.z = pos->tile.z & WORLD_CHUNK_MASK;
		return result;
	}

	// TODO @Important: It may take some extra blocks of memory
	// Because entities are spreading across half-filled resident blocks.
	// When entity goes from half-filled block to full it need extra block.
	// Number of extra blocks should be fixed and depends of number of chunks and
	// entities
	WorldPosition
	ChangeEntityPos(World* world, StoredEntity* entity,
					WorldPosition newPos, WorldPosition camTargetWorldPos,
					MemoryArena* arena)
	{
		WorldPosition oldPos = entity->worldPos;
		AB_ASSERT(IsNormalized(&oldPos));
		if (!IsNormalized(&newPos))
		{
			newPos = NormalizeWorldPos(&newPos);
		}

		ChunkPosition oldChunkCoord = GetChunkPos(&oldPos);
		ChunkPosition newChunkCoord = GetChunkPos(&newPos);

		if((oldChunkCoord.chunk.x == newChunkCoord.chunk.x) &&
		   (oldChunkCoord.chunk.y == newChunkCoord.chunk.y) &&
		   (oldChunkCoord.chunk.z == newChunkCoord.chunk.z))
		{
			// NOTE: Are in the same chunk
		}
		else
		{
			// NOTE: Chunk transition
			// Here where fun begins!!!
			Chunk* oldChunk = GetChunk(world, oldChunkCoord.chunk);
			Chunk* newChunk = GetChunk(world, newChunkCoord.chunk);
			
			AB_ASSERT(oldChunk);
			AB_ASSERT(newChunk);
			{
				EntityBlock* oldBlock = &oldChunk->firstEntityBlock;
				EntityBlock* prevBlock = nullptr;
				bool found = false;
				do
				{
					for (u32 i = 0; i < oldBlock->count; i++)
					{
						if (oldBlock->lowEntityIndices[i] == entity->storage.id)
						{
							// NOTE: Moving last index from last block here
							EntityBlock* head = &oldChunk->firstEntityBlock;
							u32 lastIndex =
								head->lowEntityIndices[head->count - 1];
							head->count--;
							oldBlock->lowEntityIndices[i] = lastIndex;		

							// NOTE: Head block empty. Move it to freelist
							if (head->count == 0 && head->nextBlock)
							{
								EntityBlock* freeBlock = head->nextBlock;
								*head = *head->nextBlock;

								if (!world->firstFreeBlock)
								{
									world->firstFreeBlock = freeBlock;
									freeBlock->nextBlock = nullptr;
								}
								else
								{
									freeBlock->nextBlock = world->firstFreeBlock;
									world->firstFreeBlock = freeBlock;
								}
								world->freeEntityBlockCount++;
							}
							
							found = true;
							break;
						}
					}
					prevBlock = oldBlock;
					oldBlock = oldBlock->nextBlock;
				} while (oldBlock && !found);
				AB_ASSERT(found);
				AB_ASSERT(oldChunk->entityCount > 0);
				oldChunk->entityCount--;
			}
			{
				EntityBlock* newBlock = &newChunk->firstEntityBlock;
				if (newBlock->count < ENTITY_BLOCK_CAPACITY)
				{
					newBlock->lowEntityIndices[newBlock->count] =
						entity->storage.id;
					newBlock->count++;
				}
				else
				{
					EntityBlock* emptyBlock = GetEmptyEntityBlock(world, arena);
					AB_ASSERT(emptyBlock);
					*emptyBlock = newChunk->firstEntityBlock;
					newChunk->firstEntityBlock.count = 0;
					newChunk->firstEntityBlock.nextBlock = emptyBlock;
					newChunk->firstEntityBlock.count++;
					newChunk->firstEntityBlock.lowEntityIndices[0]
						= entity->storage.id;
				}
				newChunk->entityCount++;
			}
#if 0
			// TODO: Move entities in and out of sim regions
			if (oldChunk->simulated && !newChunk->simulated)
			{
				//AB_ASSERT(entity->highIndex);
				_SetEntityToLow(world, entity->highIndex);
			}
			else if (!oldChunk->simulated && newChunk->simulated)
			{
				//AB_ASSERT(!entity->highIndex);
				_SetEntityToHigh(world, camTargetWorldPos, entity->lowIndex);
			}
#endif
		}
#if 0
		HighEntity* high = GetHighEntity(world, entity->highIndex);
		if (high)
		{
			high->pos = GetCamRelPos(entity->worldPos,
									 camTargetWorldPos);
		}
#endif
		entity->worldPos = newPos;
		return newPos;
	}

	inline WorldPosition
	OffsetEntityPos(World* world, StoredEntity* entity,
					v3 offset, WorldPosition camTargetWorldPos,
					MemoryArena* arena)
	{
		WorldPosition newPos = OffsetWorldPos(entity->worldPos, offset);
		return ChangeEntityPos(world, entity, newPos, camTargetWorldPos, arena);
	}


	inline v3
	WorldPosDiff(WorldPosition* a, WorldPosition* b)
	{
		v3 result;
		// TODO: Potential integer overflow
		i32 dX = a->tile.x - b->tile.x;
		i32 dY = a->tile.y - b->tile.y;
		i32 dZ = a->tile.z - b->tile.z;
		f32 dOffX = a->offset.x - b->offset.x;
		f32 dOffY = a->offset.y - b->offset.y;
		f32 dOffZ = a->offset.z - b->offset.z;
		result = V3(WORLD_TILE_SIZE * dX + dOffX,
					WORLD_TILE_SIZE * dY + dOffY,
					WORLD_TILE_SIZE * dZ + dOffZ);
		return result;
	}


	inline v3
	GetRelativePos(WorldPosition* origin, WorldPosition* target)
	{
		v3 result = WorldPosDiff(target, origin);
		return result;
	}

	// TODO: Make that world and renderer have same coords
	inline v3
	FlipYZ(v3 worldCoord)
	{
		v3 result;
		result.x = worldCoord.x;
		result.y = worldCoord.z;
		result.z = worldCoord.y;
		return result;
	}

	RaycastResult
	RayAABBIntersection(v3 rayFrom, v3 rayDir, v3 boxMin, v3 boxMax)
	{
		f32 tMin = 0.0f;
		bool hit = false;
		v3 normal = V3(0.0f);
		
		if (AbsF32(rayDir.x) > FLOAT_EPS)
		{
			f32 t = (boxMin.x - rayFrom.x) / rayDir.x;
			if (t >= 0.0f)
			{
				f32 yMin = rayFrom.y + rayDir.y * t;
				f32 zMin = rayFrom.z + rayDir.z * t;
				if (yMin >= boxMin.y && yMin <= boxMax.y &&
					zMin >= boxMin.z && zMin <= boxMax.z)
				{
					if (!hit || t < tMin)
					{
						tMin = t;
						normal = V3(-1.0f, 0.0f, 0.0f);
					}
					hit = true;
				}
			}
		}

		if (AbsF32(rayDir.x) > FLOAT_EPS)
		{
			f32 t = (boxMax.x - rayFrom.x) / rayDir.x;
			if (t >= 0.0f)
			{
				f32 yMax = rayFrom.y + rayDir.y * t;
				f32 zMax = rayFrom.z + rayDir.z * t;
				if (yMax >= boxMin.y && yMax <= boxMax.y &&
					zMax >= boxMin.z && zMax <= boxMax.z)
				{
					if (!hit || t < tMin)
					{
						tMin = t;
						normal = V3(1.0f, 0.0f, 0.0f);
					}

					hit = true;
				}
			}
		}

		if (AbsF32(rayDir.y) > FLOAT_EPS)
		{
			f32 t = (boxMin.y - rayFrom.y) / rayDir.y;
			if (t >= 0.0f)
			{
				f32 xMin = rayFrom.x + rayDir.x * t;
				f32 zMin = rayFrom.z + rayDir.z * t;
				if (xMin >= boxMin.x && xMin <= boxMax.x &&
					zMin >= boxMin.z && zMin <= boxMax.z)
				{
					if (!hit || t < tMin)
					{
						tMin = t;
						normal = V3(0.0f, -1.0f, 0.0f);
					}

					hit = true;
				}
			}
		}

		if (AbsF32(rayDir.y) > FLOAT_EPS)
		{
			f32 t = (boxMax.y - rayFrom.y) / rayDir.y;
			if (t >= 0.0f)
			{
				f32 xMax = rayFrom.x + rayDir.x * t;
				f32 zMax = rayFrom.z + rayDir.z * t;
				if (xMax >= boxMin.x && xMax <= boxMax.x &&
					zMax >= boxMin.z && zMax <= boxMax.z)
				{
					if (!hit || t < tMin)
					{
						tMin = t;
						normal = V3(0.0f, 1.0f, 0.0f);
					}

					hit = true;
				}
			}
		}

		if (AbsF32(rayDir.z) > FLOAT_EPS)
		{
			f32 t = (boxMin.z - rayFrom.z) / rayDir.z;
			if (t >= 0.0f)
			{
				f32 xMin = rayFrom.x + rayDir.x * t;
				f32 yMin = rayFrom.y + rayDir.y * t;
				if (xMin >= boxMin.x && xMin <= boxMax.x &&
					yMin >= boxMin.y && yMin <= boxMax.y)
				{
					if (!hit || t < tMin)
					{
						tMin = t;
						normal = V3(0.0f, 0.0f, -1.0f);
					}

					hit = true;
				}
			}
		}

		if (AbsF32(rayDir.z) > FLOAT_EPS)
		{
			f32 t = (boxMax.z - rayFrom.z) / rayDir.z;
			if (t >= 0.0f)
			{
				f32 xMax = rayFrom.x + rayDir.x * t;
				f32 yMax = rayFrom.y + rayDir.y * t;
				if (xMax >= boxMin.x && xMax <= boxMax.x &&
					yMax >= boxMin.y && yMax <= boxMax.y)
				{
					if (!hit || t < tMin)
					{
						tMin = t;
						normal = V3(0.0f, 0.0f, 1.0f);
					}

					hit = true;
				}
			}
		}
		
		return {hit, tMin, normal};
	}


	//NOTE: Ray coordinates should be in sim region space 
	static u32 // NOTE: Entity ID
	RaycastEntities(SimRegion* region, v3 from, v3 dir)
	{
		u32 colliderID = 0;
		f32 tMin = 0.0f;
		for (u32 index = 0; index < region->entityCount; index++)
		{
			Entity* entity = region->entities + index;
			AB_ASSERT(entity);

			f32 tMinForEntity = 0.0f;
			bool intersects = false;

			for (u32 aabbIndex = 0; aabbIndex < entity->meshCount; aabbIndex++)
			{
				BBoxAligned aabb = entity->meshes[aabbIndex]->aabb;
				v3 minCorner = aabb.min * entity->size;
				v3 maxCorner = aabb.max * entity->size;
			
				minCorner = FlipYZ(minCorner);
				maxCorner = FlipYZ(maxCorner);

				minCorner += entity->pos;
				maxCorner += entity->pos;

				RaycastResult result =
					RayAABBIntersection(from, dir, minCorner, maxCorner);
				if (result.hit)
				{
					if (!intersects || tMinForEntity < result.tMin)
					{
						intersects = true;
						tMinForEntity = result.tMin;
					}
				}
			}
			if (intersects)
			{
				colliderID = entity->id;
			}

		}
		return colliderID;
	}


	//NOTE: Ray coordinates should be in sim region space
	TilemapRaycastResult
	TilemapRaycast(SimRegion* region, v3 from, v3 dir)
	{
		// TODO: Sparseness
		// STUDY: octrees and voxel cone tracing
		TilemapRaycastResult result = {};
		for (u32 chunkIndex = 0; chunkIndex < region->chunkCount; chunkIndex++)
		{
			Chunk* chunk = region->chunks[chunkIndex];
			for (u32 tileZ = 0; tileZ < WORLD_CHUNK_DIM_TILES; tileZ++)
			{
				for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
				{
					for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
					{
						TerrainTileData tile = GetTerrainTile(chunk, V3U(tileX,
																		 tileY, tileZ));
						if (tile.type)
						{
							WorldPosition tileWorldPos = {};
							tileWorldPos.tile.x = chunk->coordX * WORLD_CHUNK_DIM_TILES;
							tileWorldPos.tile.y = chunk->coordY * WORLD_CHUNK_DIM_TILES;
							tileWorldPos.tile.z = chunk->coordZ * WORLD_CHUNK_DIM_TILES;
					
							tileWorldPos.tile.x += tileX;
							tileWorldPos.tile.y += tileY;
							tileWorldPos.tile.z += tileZ;

							v3 tileSimPos = GetRelativePos(&region->origin, &tileWorldPos);
							v3 minCorner = tileSimPos;
							v3 maxCorner = tileSimPos + WORLD_TILE_SIZE;

							RaycastResult r =
								RayAABBIntersection(from, dir, minCorner, maxCorner);
					
							if (r.hit)
							{
								if (!result.hit || r.tMin < result.tMin)
								{
									result.hit = true;
									result.tMin = r.tMin;
									result.normal = r.normal;
									result.coord = GetChunkPos(&tileWorldPos);
								}
							}
						}
					
					}
				}
			}
		}
		return result;
	}
}
