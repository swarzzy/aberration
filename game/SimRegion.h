#pragma once
#include "World.h"

namespace AB
{
	// NOTE: Must be power of two!
	const u32 SIM_REGION_HASH_TABLE_SIZE = 4096;

	struct EntityHashBucket
	{
		u32 id;
		Entity* ptr;
	};
	
    struct SimRegion
    {
        WorldPosition origin;
		// NOTE: In chunks
        v3i minBound;
		v3i maxBound;
		v3i dormantMinBound;
		v3i dormantMaxBound;
        u32 maxEntityCount;
		u32 maxDormantEntityCount;
		u32 chunkCount;
		u32 dormantChunkCount;
		u32 entityCount;
		u32 dormantEntityCount;
		Chunk** chunks;
		Chunk** dormantChunks;
        Entity* entities;
		Entity* dormantEntities;

		EntityHashBucket hashTable[SIM_REGION_HASH_TABLE_SIZE];
    };
}
