#include "SimRegion.h"
#include "Memory.h"

namespace AB
{
	static EntityHashBucket*
	FindEntityBucket(SimRegion* region, u32 id)
	{
		AB_ASSERT(id);
		EntityHashBucket* result = nullptr;
		u32 hash = id;
		for (u32 offset = 0; offset < SIM_REGION_HASH_TABLE_SIZE; offset++)
		{
			u32 bucketIndex = (hash + offset) & (SIM_REGION_HASH_TABLE_SIZE - 1);
			EntityHashBucket* bucket = region->hashTable + bucketIndex;
			if ((bucket->id == id) || (bucket->id == 0))
			{
				result = bucket;
				break;
			}
		}
		return result;
	}

	inline Entity*
	GetSimEntity(SimRegion* region, u32 id)
	{
		Entity* result = nullptr;
		if (id)
		{
			EntityHashBucket* bucket = FindEntityBucket(region, id);
			AB_ASSERT(bucket);
			result = bucket->ptr;
		}
		return result;
	}

	static void
	MapSimEntityToID(SimRegion* region, u32 id, Entity* simEntity)
	{
		AB_ASSERT(id);
		EntityHashBucket* bucket = FindEntityBucket(region, id);
		AB_ASSERT(bucket || (bucket->id == 0) || (bucket->id == id));
		bucket->id = id;
		bucket->ptr = simEntity;
	}
	
	inline WorldPosition
	GetWorldPos(SimRegion* region, v3 simSpacePos)
	{
		WorldPosition result = OffsetWorldPos(region->origin, simSpacePos);
		return result;
	}
	
	inline v3
	GetSimSpacePos(SimRegion* region, WorldPosition* pos)
	{
		v3 result = WorldPosDiff(pos, &region->origin);
		return result;	
	}

	static void
	ChunkBeginSim(SimRegion* region, World* world, Chunk* chunk, SimulationType simType)
	{
		EntityBlock* block = &chunk->firstEntityBlock;
		do
		{
			for (u32 i = 0; i < block->count; i++)
			{
				u32 index = block->lowEntityIndices[i];
				StoredEntity* stored = GetStoredEntity(world, index);
				stored->storage.simType = simType;

				Entity* simEntity;
				switch(simType)
				{
				case SIM_TYPE_ACTIVE:
				{
					AB_ASSERT(region->entityCount < region->maxEntityCount);
					simEntity = region->entities + region->entityCount;
					region->entityCount++;
				} break;
				case SIM_TYPE_DORMANT:
				{
					AB_ASSERT(region->dormantEntityCount < region->maxDormantEntityCount);
					simEntity =	region->dormantEntities + region->dormantEntityCount;
					region->dormantEntityCount++;
				} break;
				INVALID_DEFAULT_CASE;
				}

				*simEntity = stored->storage;
				if (!simEntity->tiled)
				{
					simEntity->pos = GetSimSpacePos(region, &stored->worldPos);
					// TODO: Should this be invalid?
					simEntity->tileOriginPos = InvalidChunkPos();
				}
				else
				{
					simEntity->pos = V3(INVALID_REL_COORD);
					// TODO: Should this be invalid?
					simEntity->tileOriginPos = GetChunkPos(&stored->worldPos);
				}
				// TODO: This should be done when entity added to storage
				simEntity->id = index;
				MapSimEntityToID(region, index, simEntity);
			}
			block = block->nextBlock;
		}
		while(block);
		chunk->simType = simType;
		//chunk->dirty = true;
	}

	SimRegion*
	BeginSim(MemoryArena* tempArena, World* world,
			 WorldPosition origin, v3i chunkSpan)
	{
		SimRegion* region =
			(SimRegion*)PushSize(tempArena, sizeof(SimRegion), alignof(SimRegion));
		AB_ASSERT(region);
		SetZeroScalar(SimRegion, region);


		ChunkPosition chunkOrigin = GetChunkPos(&origin);
		ChunkRegion area = ChunkRegionFromOriginAndSpan(chunkOrigin.chunk, chunkSpan);

		ChunkRegion dormantArea = ChunkRegionFromOriginAndSpan(chunkOrigin.chunk, chunkSpan + 1);

		region->origin = origin;
		region->minBound = area.minBound;
		region->maxBound = area.maxBound;
		region->dormantMinBound = dormantArea.minBound;
		region->dormantMaxBound = dormantArea.maxBound;
		
		u32 entityCount = 0;
		u32 chunkCount = 0;

		u32 dormantEntityCount = 0;
		u32 dormantChunkCount = 0;

		for (i32 chunkZ = dormantArea.minBound.z; chunkZ <= dormantArea.maxBound.z; chunkZ++)
		{
			for (i32 chunkY = dormantArea.minBound.y; chunkY <= dormantArea.maxBound.y; chunkY++)
			{
				for (i32 chunkX = dormantArea.minBound.x; chunkX <= dormantArea.maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, V3I(chunkX, chunkY, chunkZ));
					if (chunk)
					{
						if (InChunkRegion(&area, V3I(chunkX, chunkY, chunkZ)))
						{
							entityCount += chunk->entityCount;
							chunkCount++;

						}
						else
						{
							dormantEntityCount += chunk->entityCount;
							dormantChunkCount++;
						}
					}
				}
			}
		}
		
		region->maxEntityCount = entityCount;
		region->maxDormantEntityCount = dormantEntityCount;

		region->entities = (Entity*)PushSize(tempArena,
											 sizeof(Entity) * entityCount,
											 alignof(Entity));
		region->dormantEntities = (Entity*)PushSize(tempArena,
													sizeof(Entity) * dormantEntityCount,
													alignof(Entity));

		// TODO: Think about making this array in place with fixed size
		region->chunks = (Chunk**)PushSize(tempArena,
										   sizeof(Chunk*) * chunkCount,
										   alignof(Chunk*));
		
		region->dormantChunks = (Chunk**)PushSize(tempArena,
												  sizeof(Chunk*) * dormantChunkCount,
												  alignof(Chunk*));

		AB_ASSERT(region->entities);
		AB_ASSERT(region->dormantEntities);
		AB_ASSERT(region->chunks);
		AB_ASSERT(region->dormantChunks);
		
		SetZeroArray(Entity, entityCount, region->entities);
		SetZeroArray(Entity, entityCount, region->dormantChunks);

		for (i32 chunkZ = dormantArea.minBound.z; chunkZ <= dormantArea.maxBound.z; chunkZ++)
		{
			for (i32 chunkY = dormantArea.minBound.y; chunkY <= dormantArea.maxBound.y; chunkY++)
			{
				for (i32 chunkX = dormantArea.minBound.x; chunkX <= dormantArea.maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, V3I(chunkX, chunkY, chunkZ));
					if (chunk)
					{
						AB_ASSERT(!(chunk->simType));
						// TODO: Flag in entity and chunks that indicates if it is
						// dormant or not
						
						if (InChunkRegion(&area, V3I(chunkX, chunkY, chunkZ)))
						{
							ChunkBeginSim(region, world, chunk, SIM_TYPE_ACTIVE);
							region->chunks[region->chunkCount] = chunk;
							region->chunkCount++;
						}
						else
						{
							ChunkBeginSim(region, world, chunk, SIM_TYPE_DORMANT);
							region->dormantChunks[region->dormantChunkCount] = chunk;
							region->dormantChunkCount++;
						}
					}
				}
			}
		}
		AB_ASSERT(region->chunkCount == chunkCount);
		AB_ASSERT(region->dormantChunkCount == dormantChunkCount);
		AB_ASSERT(region->entityCount == region->maxEntityCount);
		AB_ASSERT(region->dormantEntityCount == region->maxDormantEntityCount);
		
		return region;
	}

	void
	EndSim(SimRegion* region, World* world, MemoryArena* arena)
	{
		for (u32 simIndex = 0; simIndex < region->entityCount; simIndex++)
		{
			Entity* sim = region->entities + simIndex;
			sim->simType = SIM_TYPE_INACTIVE;
			StoredEntity* stored = GetStoredEntity(world, sim->id);
			WorldPosition newPos;
			if (!sim->tiled)
			{
				newPos = OffsetWorldPos(region->origin, sim->pos);
			}
			else
			{
				newPos = GetWorldPos(&sim->tileOriginPos);
			}
			stored->worldPos = ChangeEntityPos(world, stored, newPos,
											   region->origin, arena);
			stored->storage = *sim;			
		}
		
		for (u32 simIndex = 0; simIndex < region->dormantEntityCount; simIndex++)
		{
			Entity* sim = region->dormantEntities + simIndex;
			StoredEntity* stored = GetStoredEntity(world, sim->id);

			stored->storage.simType = SIM_TYPE_INACTIVE;
		}

		v3i minBound = region->dormantMinBound;
		v3i maxBound = region->dormantMaxBound;
		
		for (i32 chunkZ = minBound.z; chunkZ <= maxBound.z; chunkZ++)
		{
			for (i32 chunkY = minBound.y; chunkY <= maxBound.y; chunkY++)
			{
				for (i32 chunkX = minBound.x; chunkX <= maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, V3I(chunkX, chunkY, chunkZ));
					if (chunk)
					{
						AB_ASSERT(chunk->simType);
						chunk->simType = SIM_TYPE_INACTIVE;
						//chunk->dirty = true;
					}
				}
			}
		}		
	}
}
