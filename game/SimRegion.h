#pragma once
#include "World.h"

namespace AB
{
	// NOTE: Must be power of two!
	const u32 SIM_REGION_HASH_TABLE_SIZE = 4096;

	struct SimEntityHashBucket
	{
		u32 id;
		SimEntity* ptr;
	};
	
    struct SimRegion
    {
        WorldPosition origin;
		// NOTE: In chunks
        v3i minBound;
		v3i maxBound;
        u32 entityCount;
        u32 maxEntityCount;
		u32 chunkCount;
		Chunk** chunks;
        SimEntity* entities;

		SimEntityHashBucket hashTable[SIM_REGION_HASH_TABLE_SIZE];
    };
}
