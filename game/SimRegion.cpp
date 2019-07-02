#include "SimRegion.h"
#include "Memory.h"

namespace AB
{
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
				LowEntity* storage = GetLowEntity(world, index);
				AB_ASSERT(region->entityCount < region->maxEntityCount);
				SimEntity* simEntity = region->entities + region->entityCount;
				region->entityCount++;
				
				simEntity->pos = GetSimSpacePos(region, &storage->worldPos);
				simEntity->stored = storage;
			}
			block = block->nextBlock;
		}
		while(block);
		chunk->simulated = true;
		chunk->dirty = true;
	}

	SimRegion* BeginSim(MemoryArena* tempArena, World* world,
						WorldPosition origin, v3i span)
	{
		SimRegion* region =
			(SimRegion*)PushSize(tempArena, sizeof(SimRegion), alignof(SimRegion));
		AB_ASSERT(region);
		SetZeroScalar(SimRegion, region);

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
					}
				}
			}
		}
		AB_ASSERT(region->entityCount == region->maxEntityCount);
		return region;
	}

	void EndSim(SimRegion* region, World* world, MemoryArena* arena)
	{
		for (u32 simIndex = 0; simIndex < region->entityCount; simIndex++)
		{
			SimEntity* entity = region->entities + simIndex;
			LowEntity* stored = entity->stored;
			stored->worldPos = OffsetEntityPos(world, stored, entity->pos,
											   region->origin, arena);
			
		}

		v3i minBound = region->minBound;
		v3i maxBound = region->maxBound;
		
		for (i32 chunkZ = minBound.x; chunkZ <= maxBound.z; chunkZ++)
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
						chunk->dirty = true;
					}
				}
			}
		}		
	}
}
