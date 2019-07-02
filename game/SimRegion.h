#pragma once

namespace AB
{
    struct SimEntity
    {
        v3 pos;
        LowEntity* stored;
    };

    struct SimRegion
    {
        WorldPosition origin;
		// NOTE: In chunks
        v3i minBound;
		v3i maxBound;
        u32 entityCount;
        u32 maxEntityCount;
        SimEntity* entities;
    };
}
