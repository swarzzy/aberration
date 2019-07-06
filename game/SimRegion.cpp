#include "SimRegion.h"
#include "Memory.h"

namespace AB
{
	static SimEntityHashBucket* FindEntityBucket(SimRegion* region, u32 id)
	{
		AB_ASSERT(id);
		SimEntityHashBucket* result = nullptr;
		u32 hash = id;
		for (u32 offset = 0; offset < SIM_REGION_HASH_TABLE_SIZE; offset++)
		{
			u32 bucketIndex = (hash + offset) & (SIM_REGION_HASH_TABLE_SIZE - 1);
			SimEntityHashBucket* bucket = region->hashTable + bucketIndex;
			if ((bucket->id == id) || (bucket->id == 0))
			{
				result = bucket;
				break;
			}
		}
		return result;
	}

	inline SimEntity* GetSimEntity(SimRegion* region, u32 id)
	{
		SimEntity* result = nullptr;
		if (id)
		{
			SimEntityHashBucket* bucket = FindEntityBucket(region, id);
			AB_ASSERT(bucket);
			result = bucket->ptr;
		}
		return result;
	}

	static void MapSimEntityToID(SimRegion* region, u32 id, SimEntity* simEntity)
	{
		AB_ASSERT(id);
		SimEntityHashBucket* bucket = FindEntityBucket(region, id);
		AB_ASSERT(bucket || (bucket->id == 0) || (bucket->id == id));
		bucket->id = id;
		bucket->ptr = simEntity;
	}
	
	inline WorldPosition GetWorldPos(SimRegion* region, v3 simSpacePos)
	{
		WorldPosition result = OffsetWorldPos(region->origin, simSpacePos);
		return result;
	}
	
	inline v3 GetSimSpacePos(SimRegion* region, WorldPosition* pos)
	{
		v3 result = WorldPosDiff(*pos, region->origin);
		return result;	
	}

	static void ChunkBeginSim(SimRegion* region, World* world, Chunk* chunk)
	{
		EntityBlock* block = &chunk->firstEntityBlock;
		do
		{
			for (u32 i = 0; i < block->count; i++)
			{
				u32 index = block->lowEntityIndices[i];
				LowEntity* stored = GetLowEntity(world, index);
				AB_ASSERT(region->entityCount < region->maxEntityCount);
				SimEntity* simEntity = region->entities + region->entityCount;
				region->entityCount++;

				*simEntity = stored->stored;
				simEntity->pos = GetSimSpacePos(region, &stored->worldPos);
				// TODO: This should be done when entity added to storage
				simEntity->id = index;
				MapSimEntityToID(region, index, simEntity);
			}
			block = block->nextBlock;
		}
		while(block);
		chunk->simulated = true;
		//chunk->dirty = true;
	}

	SimRegion* BeginSim(MemoryArena* tempArena, World* world,
						WorldPosition origin, v3i span)
	{
		SimRegion* region =
			(SimRegion*)PushSize(tempArena, sizeof(SimRegion), alignof(SimRegion));
		AB_ASSERT(region);
		SetZeroScalar(SimRegion, region);

		u32 chunkCount = (span.x * 2 + 1) * (span.y * 2 + 1) * (span.z * 2 + 1);

		v3i minBound;
		v3i maxBound;

		minBound.x = SafeSubChunkCoord(origin.chunkX, span.x);
		minBound.y = SafeSubChunkCoord(origin.chunkY, span.y);
		minBound.z = SafeSubChunkCoord(origin.chunkZ, span.z);
		maxBound.x = SafeAddChunkCoord(origin.chunkX, span.x);
		maxBound.y = SafeAddChunkCoord(origin.chunkY, span.y);
		maxBound.z = SafeAddChunkCoord(origin.chunkZ, span.z);

		region->origin = origin;
		region->minBound = minBound;
		region->maxBound = maxBound;
		
		u32 entityCount = 0;

		for (i32 chunkZ = minBound.x; chunkZ <= maxBound.z; chunkZ++)
		{
			for (i32 chunkY = minBound.y; chunkY <= maxBound.y; chunkY++)
			{
				for (i32 chunkX = minBound.x; chunkX <= maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY, chunkZ);
					if (chunk)
					{
						entityCount += chunk->entityCount;
					}
				}
			}
		}

		region->maxEntityCount = entityCount;

		region->entities = (SimEntity*)PushSize(tempArena,
												sizeof(SimEntity) * entityCount,
												alignof(SimEntity));
		region->chunks = (Chunk**)PushSize(tempArena,
										  sizeof(Chunk*) * chunkCount,
										  alignof(Chunk*));
		AB_ASSERT(region->entities);
		SetZeroArray(SimEntity, entityCount, region->entities);

		for (i32 chunkZ = minBound.z; chunkZ <= maxBound.z; chunkZ++)
		{
			for (i32 chunkY = minBound.y; chunkY <= maxBound.y; chunkY++)
			{
				for (i32 chunkX = minBound.x; chunkX <= maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY, chunkZ);
					if (chunk)
					{
						AB_ASSERT(!(chunk->simulated));
						ChunkBeginSim(region, world, chunk);
						region->chunks[region->chunkCount] = chunk;
						region->chunkCount++;
					}
				}
			}
		}
		AB_ASSERT(region->chunkCount == chunkCount);
		AB_ASSERT(region->entityCount == region->maxEntityCount);
		return region;
	}

	void EndSim(SimRegion* region, World* world, MemoryArena* arena)
	{
		for (u32 simIndex = 0; simIndex < region->entityCount; simIndex++)
		{
			SimEntity* sim = region->entities + simIndex;
			LowEntity* stored = GetLowEntity(world, sim->id);
			WorldPosition newPos = OffsetWorldPos(region->origin, sim->pos);
			stored->worldPos = ChangeEntityPos(world, stored, newPos,
											   region->origin, arena);
			stored->stored = *sim;
			
		}

		v3i minBound = region->minBound;
		v3i maxBound = region->maxBound;
		
		for (i32 chunkZ = minBound.z; chunkZ <= maxBound.z; chunkZ++)
		{
			for (i32 chunkY = minBound.y; chunkY <= maxBound.y; chunkY++)
			{
				for (i32 chunkX = minBound.x; chunkX <= maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY, chunkZ);
					if (chunk)
					{
						AB_ASSERT(chunk->simulated);
						chunk->simulated = false;
						//chunk->dirty = true;
					}
				}
			}
		}		
	}
}
