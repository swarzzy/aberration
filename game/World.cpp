#include "World.h"

namespace AB
{
	inline i32
	SafeAddChunkCoord(i32 a, i32 b);
	

	inline i32
	SafeSubChunkCoord(i32 a, i32 b)
	{
		i32 result = 0;
		if (b < 0)
		{
			result = SafeAddChunkCoord(a, -b);
		}
		else
		{
			result = a - b;
			if (result > a)
			{
				result = AB_INT32_MIN + CHUNK_SAFE_MARGIN;
			}
		}
		
		return result;
	}

	inline i32
	SafeAddChunkCoord(i32 a, i32 b)
	{
		i32 result;
		if (b < 0)
		{
			result = SafeSubChunkCoord(a, -b);
		}
		else
		{
			result = a + b;
			if (result < a)
			{
				result = AB_INT32_MAX - CHUNK_SAFE_MARGIN;
			}
		}

		return result;
	}

	World*
	CreateWorld(MemoryArena* arena)
	{
		World* world = nullptr;
		world = (World*)PushSize(arena, sizeof(World), alignof(World));
		SetZeroScalar(World, world);
		
		world->tileSizeRaw = 3.0f;
		world->tileSizeInUnits = 1.0f;
		world->tileRadiusInUnits = 0.5f;
		world->chunkSizeUnits = world->tileSizeInUnits * WORLD_CHUNK_DIM_TILES;
		world->toUnits = world->tileSizeInUnits / world->tileSizeRaw;
		world->unitsToRaw = world->tileSizeRaw / world->tileSizeInUnits;

		for (u32 i = 0; i < CHUNK_TABLE_SIZE; i++)
		{
			world->chunkTable[i].coordX = INVALID_CHUNK_COORD;
			world->chunkTable[i].coordY = INVALID_CHUNK_COORD;
		}

		return world;
	}

	inline LowEntity*
	GetLowEntity(World* world, u32 lowIndex)
	{
		LowEntity* result = nullptr;
		if (lowIndex > 0 && lowIndex <= world->lowEntityCount)
		{
			result = world->lowEntities + lowIndex;
		}
		return result;		
	}

	inline HighEntity*
	GetHighEntity(World* world, u32 highIndex)
	{
		HighEntity* result = nullptr;
		if (highIndex > 0 && highIndex <= world->highEntityCount)
		{
			result = world->highEntities + highIndex;
		}
		return result;
	}

	
	inline Entity
	GetEntityFromLowIndex(World* world, u32 lowIndex)
	{
		Entity result = {};
		if (lowIndex > 0 && lowIndex <= world->lowEntityCount)
		{
			result.low = world->lowEntities + lowIndex;
			
			u32 highIndex = result.low->highIndex;
			
			if (highIndex > 0 && highIndex <= world->highEntityCount)
			{
				result.high = world->highEntities + highIndex;
			}
		}
		
		return result;
	}

	inline Entity
	GetEntityFromHighIndex(World* world, u32 highIndex)
	{
		Entity result = {};
		if (highIndex > 0 && highIndex <= world->highEntityCount)
		{
			result.high = world->highEntities + highIndex;
			result.low = world->lowEntities + result.high->lowIndex;
		}
		
		return result;
	}
	
	u32
	_SetEntityToHigh(World* world, u32 lowIndex)
	{
		AB_ASSERT(lowIndex < MAX_LOW_ENTITIES);
		u32 result = 0;
		LowEntity* low = world->lowEntities + lowIndex;
		if (!low->highIndex)
		{
			if (world->highEntityCount < MAX_HIGH_ENTITIES)
			{
				world->highEntityCount++;				

				HighEntity* high =
					world->highEntities + world->highEntityCount;

				//high->pos = WorldPosDiff(world,
				//						 low->worldPos,
				//						 camera->targetWorldPos);
				//high->velocity = V2(0.0f);
				high->lowIndex = lowIndex;
				low->highIndex = world->highEntityCount;
				result = world->highEntityCount;
			}
			else
			{
				INVALID_CODE_PATH();
			}
		}
		return result;
	}

	void
	_SetEntityToLow(World* world, u32 highIndex)
	{
		AB_ASSERT(highIndex < MAX_HIGH_ENTITIES);
		HighEntity* high = GetHighEntity(world, highIndex);
		LowEntity* low = GetLowEntity(world, high->lowIndex);

		u32 lastEntityIndex = world->highEntityCount;
		u32 currentEntityIndex = low->highIndex;
		
		//low->worldPos =
		//	ChangeWorldPosition(world,
		//						camera->targetWorldPos,
		//						high->pos);
		low->highIndex = 0;

		if (!(currentEntityIndex == lastEntityIndex))
		{
			Entity lastEntity = GetEntityFromHighIndex(world, lastEntityIndex);
			*(high) = *(lastEntity.high);
			lastEntity.low->highIndex = currentEntityIndex;
		}
		world->highEntityCount--;
	}

	inline Chunk*
	GetChunk(World* world, i32 chunkX, i32 chunkY, MemoryArena* arena = nullptr)
	{
		Chunk* result = nullptr;
		// TODO: Check is chunk coord are valid
		// TODO: Better hash function !!111
		u32 chunkHash = (AbsI32(chunkX) * 22 + AbsI32(chunkY) * 12)
			% CHUNK_TABLE_SIZE;
		AB_ASSERT(chunkHash < CHUNK_TABLE_SIZE);

		Chunk* currentChunk = world->chunkTable + chunkHash;
		if (currentChunk->coordX != INVALID_CHUNK_COORD)
		{
			if (currentChunk->coordX == chunkX && currentChunk->coordY == chunkY)
			{
				result = currentChunk;
			}
			else
			{
				while (currentChunk->nextChunk)
				{
					Chunk* nextChunk = currentChunk->nextChunk;
					if (nextChunk->coordX == chunkX &&
						nextChunk->coordY == chunkY)
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
			if (currentChunk->coordX == INVALID_CHUNK_COORD)
			{
				newChunk = currentChunk;
			}
			else
			{
				newChunk = (Chunk*)PushSize(arena, sizeof(Chunk),
											alignof(Chunk));
				currentChunk->nextChunk = newChunk;
			}

			newChunk->coordX = chunkX;
			newChunk->coordY = chunkY;
			newChunk->nextChunk = nullptr;
			world->chunkCount++;

			result = newChunk;
		}
		
		return result;
	}
	
	inline TerrainType
	GetTerrainTile(Chunk* chunk, u32 tileInChunkX, u32 tileInChunkY)
	{
		TerrainType result = TERRAIN_TYPE_EMPTY;
		
		AB_ASSERT(tileInChunkX < WORLD_CHUNK_DIM_TILES);
		AB_ASSERT(tileInChunkY < WORLD_CHUNK_DIM_TILES);
		
		if (chunk)
		{
			result = chunk->terrainTiles[tileInChunkY * WORLD_CHUNK_DIM_TILES
										 + tileInChunkX];
		}
		
		return result;
	}
	
	inline void
	SetTerrainTile(Chunk* chunk, u32 tileX, u32 tileY, TerrainType type)
	{
		AB_ASSERT(tileX < WORLD_CHUNK_DIM_TILES);
		AB_ASSERT(tileY < WORLD_CHUNK_DIM_TILES);

		chunk->terrainTiles[tileY * WORLD_CHUNK_DIM_TILES + tileX] = type;		
	}
	
	inline WorldPosition
	OffsetWorldPos(World* world, WorldPosition oldPos, v2 offset)
	{
		// TODO: Better checking!
		WorldPosition newPos = oldPos;
		newPos.offset += offset;
		i32 chunkOffsetX = Floor(newPos.offset.x / world->chunkSizeUnits);
		i32 chunkOffsetY = Floor(newPos.offset.y / world->chunkSizeUnits);
		newPos.offset.x -= chunkOffsetX * world->chunkSizeUnits;
		newPos.offset.y -= chunkOffsetY * world->chunkSizeUnits;

		AB_ASSERT(oldPos.chunkX + chunkOffsetX > INT32_MIN + CHUNK_SAFE_MARGIN);
		AB_ASSERT(oldPos.chunkX + chunkOffsetX < INT32_MAX - CHUNK_SAFE_MARGIN);

		AB_ASSERT(oldPos.chunkY + chunkOffsetY > INT32_MIN + CHUNK_SAFE_MARGIN);
		AB_ASSERT(oldPos.chunkY + chunkOffsetY < INT32_MAX - CHUNK_SAFE_MARGIN);

		newPos.chunkX = SafeAddChunkCoord(newPos.chunkX, chunkOffsetX);
		newPos.chunkY = SafeAddChunkCoord(newPos.chunkY, chunkOffsetY);

		return newPos;
	}

	static inline EntityBlock*
	GetEmptyEntityBlock(World* world, MemoryArena* arena = nullptr)
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

	// TODO @Important: It may take some extra blocks of memory
	// Because entities are spreading across half-filled resident blocks.
	// When entity goes from half-filled block to full it need extra block.
	// Number of extra blocks should be fixed and depends of number of chunks and
	// entities
	void
	ChangeEntityPos(World* world, LowEntity* entity,
					WorldPosition newPos, MemoryArena* arena)
	{
		WorldPosition oldPos = entity->worldPos;
		if(oldPos.chunkX == newPos.chunkX && oldPos.chunkY == newPos.chunkY)
		{
			// NOTE: In same chunk
		}
		else
		{
			// NOTE: Chunk transition
			// Here where fun begins!!!
			Chunk* oldChunk = GetChunk(world, oldPos.chunkX, oldPos.chunkY);
			Chunk* newChunk = GetChunk(world, newPos.chunkX, newPos.chunkY);
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
						if (oldBlock->lowEntityIndices[i] == entity->lowIndex)
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
			}
			{
				EntityBlock* newBlock = &newChunk->firstEntityBlock;
				if (newBlock->count < ENTITY_BLOCK_CAPACITY)
				{
					newBlock->lowEntityIndices[newBlock->count] =
						entity->lowIndex;
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
						= entity->lowIndex;
				}
			}
			if (oldChunk->high && !newChunk->high)
			{
				AB_ASSERT(entity->highIndex);
				_SetEntityToLow(world, entity->highIndex);
			}
			else if (!oldChunk->high && newChunk->high)
			{
				AB_ASSERT(!entity->highIndex);
				_SetEntityToHigh(world, entity->lowIndex);
			}
		}
	
		entity->worldPos = newPos;
	}

	inline v2
	WorldPosDiff(World* world, WorldPosition a, WorldPosition b)
	{
		v2 result;
		// TODO: Potential integer overflow
		i32 dX = a.chunkX - b.chunkX;
		i32 dY = a.chunkY - b.chunkY;
		f32 dOffX = a.offset.x - b.offset.x;
		f32 dOffY = a.offset.y - b.offset.y;
		result = V2(world->chunkSizeUnits * dX + dOffX,
					world->chunkSizeUnits * dY + dOffY);
		return result;
	}

	// TODO: Pass world instead of gameState
	u32
	AddLowEntity(World* world, Chunk* chunk, EntityType type,
				 MemoryArena* arena = nullptr)
	{
		AB_ASSERT(world->lowEntityCount < MAX_LOW_ENTITIES);
		u32 index = 0;
		if (chunk->firstEntityBlock.count < ENTITY_BLOCK_CAPACITY)
		{
			world->lowEntityCount++;
			index = world->lowEntityCount;
			world->lowEntities[index] = {};
			world->lowEntities[index].type = type;
			world->lowEntities[index].lowIndex = index;
		
			chunk->firstEntityBlock.lowEntityIndices[chunk->firstEntityBlock.count]
				= index;
			chunk->firstEntityBlock.count++;
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

				world->lowEntityCount++;
				index = world->lowEntityCount;
				world->lowEntities[index] = {};
				world->lowEntities[index].type = type;
				world->lowEntities[index].lowIndex = index;
		
				chunk->firstEntityBlock.lowEntityIndices[0] = index;
			}
		}

		return index;
	}
	
	inline v2
	GetCamRelPos(World* world, WorldPosition worldPos,
				 WorldPosition camTargetWorldPos)
	{
		v2 result = WorldPosDiff(world, worldPos, camTargetWorldPos);
		return result;
	}

	u32 // NOTE: LowEntityIndex
	Raycast(World* world, Camera* camera, v3 from, v3 dir)
	{
		// TODO: Just reserve null entity instead of this mess
		u32 colliderIndex = 0;
		f32 tMin = 0.0f;
		for (u32 index = 1; index <= world->highEntityCount; index++)
		{
			Entity _entity = GetEntityFromHighIndex(world, index);
			LowEntity* entity = _entity.low;
			AB_ASSERT(entity);
			
			v2 _entityCamRelPos = GetCamRelPos(world, entity->worldPos,
											   camera->targetWorldPos);
			// TODO: Full 3d position
			v3 entityCamRelPos = V3(_entityCamRelPos.x,
									world->tileSizeInUnits * 2.0f,
									_entityCamRelPos.y);

			// TODO: Complete size conversion to full 3d
			// TODO: bounding volumes
			v3 minCorner = entityCamRelPos + (-0.5f * entity->size);
			v3 maxCorner = entityCamRelPos + (0.5f * entity->size);

			f32 tMinForEntity = 0.0f;
			bool intersects = false;

			if (AbsF32(dir.x) > FLOAT_EPS)
			{
				f32 t = (minCorner.x - from.x) / dir.x;
				if (t >= 0.0f)
				{
					f32 yMin = from.y + dir.y * t;
					f32 zMin = from.z + dir.z * t;
					if (yMin >= minCorner.y && yMin <= maxCorner.y &&
						zMin >= minCorner.z && zMin <= maxCorner.z)
					{
						if (!intersects || t < tMinForEntity)
						{
							tMinForEntity = t;
						}
						intersects = true;
					}
				}
			}

			if (AbsF32(dir.x) > FLOAT_EPS)
			{
				f32 t = (maxCorner.x - from.x) / dir.x;
				if (t >= 0.0f)
				{
					f32 yMax = from.y + dir.y * t;
					f32 zMax = from.z + dir.z * t;
					if (yMax >= minCorner.y && yMax <= maxCorner.y &&
						zMax >= minCorner.z && zMax <= maxCorner.z)
					{
						if (!intersects || t < tMinForEntity)
						{
							tMinForEntity = t;
						}

						intersects = true;
					}
				}
			}

			if (AbsF32(dir.y) > FLOAT_EPS)
			{
				f32 t = (minCorner.y - from.y) / dir.y;
				if (t >= 0.0f)
				{
					f32 xMin = from.x + dir.x * t;
					f32 zMin = from.z + dir.z * t;
					if (xMin >= minCorner.x && xMin <= maxCorner.x &&
						zMin >= minCorner.z && zMin <= maxCorner.z)
					{
						if (!intersects || t < tMinForEntity)
						{
							tMinForEntity = t;
						}

						intersects = true;
					}
				}
			}

			if (AbsF32(dir.y) > FLOAT_EPS)
			{
				f32 t = (maxCorner.y - from.y) / dir.y;
				if (t >= 0.0f)
				{
					f32 xMax = from.x + dir.x * t;
					f32 zMax = from.z + dir.z * t;
					if (xMax >= minCorner.x && xMax <= maxCorner.x &&
						zMax >= minCorner.z && zMax <= maxCorner.z)
					{
						if (!intersects || t < tMinForEntity)
						{
							tMinForEntity = t;
						}

						intersects = true;
					}
				}
			}

			if (AbsF32(dir.z) > FLOAT_EPS)
			{
				f32 t = (minCorner.z - from.z) / dir.z;
				if (t >= 0.0f)
				{
					f32 xMin = from.x + dir.x * t;
					f32 yMin = from.y + dir.y * t;
					if (xMin >= minCorner.x && xMin <= maxCorner.x &&
						yMin >= minCorner.y && yMin <= maxCorner.y)
					{
						if (!intersects || t < tMinForEntity)
						{
							tMinForEntity = t;
						}

						intersects = true;
					}
				}
			}

			if (AbsF32(dir.z) > FLOAT_EPS)
			{
				f32 t = (maxCorner.z - from.z) / dir.z;
				if (t >= 0.0f)
				{
					f32 xMax = from.x + dir.x * t;
					f32 yMax = from.y + dir.y * t;
					if (xMax >= minCorner.x && xMax <= maxCorner.x &&
						yMax >= minCorner.y && yMax <= maxCorner.y)
					{
						if (!intersects || t < tMinForEntity)
						{
							tMinForEntity = t;
						}

						intersects = true;
					}
				}
			}

			if (intersects)
			{
				if (colliderIndex == 0 || tMinForEntity < tMin)
				{
					colliderIndex = index;
					tMin = tMinForEntity;
				}
			}
		}
		if (colliderIndex)
		{
			Entity entity = GetEntityFromHighIndex(world, colliderIndex);
			colliderIndex = entity.low->lowIndex;
		}
		return colliderIndex;
	}

	// TODO: Think about what's happens if offset is out if chunk bounds
	u32
	AddWallEntity(World* world, Chunk* chunk, v2 offset,
				  MemoryArena* arena = 0)
	{
		// assert for coord out of chunk bounds
		u32 index = 0;
		index = AddLowEntity(world, chunk, ENTITY_TYPE_WALL, arena);
		if (index)
		{
			world->lowEntities[index] = {
				index,
				ENTITY_TYPE_WALL,
				// TODO: Offsets that out of chunk bounds
				{chunk->coordX, chunk->coordY, offset},
				0.0f,
				V3(1.0f),
				V3(1.0f, 0.0f, 1.0f),
			};	
		}
		
		return index;
	}


}
