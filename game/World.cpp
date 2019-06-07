#include "World.h"

namespace AB
{
	World*
	CreateWorld(MemoryArena* arena)
	{
		World* world = nullptr;
		world = (World*)PushSize(arena, sizeof(World), alignof(World));
		SetZeroScalar(World, world);
		
		world->chunkCountX = 8;
		world->chunkCountY = 8;
		world->tileSizeRaw = 3.0f;
		world->tileSizeInUnits = 1.0f;
		world->tileRadiusInUnits = 0.5f;
		world->toUnits = world->tileSizeInUnits / world->tileSizeRaw;
		world->unitsToRaw = world->tileSizeRaw / world->tileSizeInUnits;

		for (u32 i = 0; i < CHUNK_TABLE_SIZE; i++)
		{
			world->chunkTable[i].coordX = INVALID_CHUNK_COORD;
			world->chunkTable[i].coordY = INVALID_CHUNK_COORD;
		}

		return world;
	}

	inline void
	RecanonicalizeCoord(f32 tileSizeUnits, i32* restrict tileCoord,
						f32* restrict coord)
	{
		i32 tileOffset = RoundF32I32(*coord / tileSizeUnits);
		
		AB_ASSERT(AbsI32(tileOffset) <
				  ((AB_INT32_MAX - AbsI32(*tileCoord)) - TILEMAP_SAFE_MARGIN));

		*tileCoord += tileOffset;
		*coord -= (f32)tileOffset * tileSizeUnits;				
	}

	inline WorldPosition
	MapToTileSpace(World* world, WorldPosition worldCenter, v2 relPos)
	{
		WorldPosition result = worldCenter;
		result.offset += relPos;
		RecanonicalizeCoord(world->tileSizeInUnits, &result.tileX,
							&result.offset.x);
		RecanonicalizeCoord(world->tileSizeInUnits, &result.tileY,
							&result.offset.y);

		AB_ASSERT(result.offset.x >= -world->tileRadiusInUnits);
		AB_ASSERT(result.offset.y >= -world->tileRadiusInUnits);
		AB_ASSERT(result.offset.x <= world->tileRadiusInUnits);
		AB_ASSERT(result.offset.y <= world->tileRadiusInUnits);

		return result;
	}

	inline WorldPosition
	RecanonicalizePosition(World* world, WorldPosition pos)
	{
		RecanonicalizeCoord(world->tileSizeInUnits, &pos.tileX, &pos.offset.x);
		RecanonicalizeCoord(world->tileSizeInUnits, &pos.tileY, &pos.offset.y);

		AB_ASSERT(pos.offset.x >= -world->tileRadiusInUnits);
		AB_ASSERT(pos.offset.y >= -world->tileRadiusInUnits);
		AB_ASSERT(pos.offset.x <= world->tileRadiusInUnits);
		AB_ASSERT(pos.offset.y <= world->tileRadiusInUnits);

		return pos;
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

			result = newChunk;
		}
		
		return result;
	}

	inline ChunkPosition
	GetChunkPosition(i32 absTileX, i32 absTileY)
	{
		ChunkPosition result;
		result.chunkX = absTileX >> WORLD_CHUNK_SHIFT;
		result.chunkY = absTileY >> WORLD_CHUNK_SHIFT;
		result.tileInChunkX = absTileX & WORLD_CHUNK_MASK;
		result.tileInChunkY = absTileY & WORLD_CHUNK_MASK;

		return result;
	}

	inline WorldPosition
	GetWorldPosition(i32 chunkX, i32 chunkY, i32 relTileX, i32 relTileY)
	{
		WorldPosition result = {};
		result.tileX = chunkX * WORLD_CHUNK_DIM_TILES;
		result.tileY = chunkY * WORLD_CHUNK_DIM_TILES;
		result.tileX += relTileX;
		result.tileY += relTileY;

		return result;		
	}

	inline WorldPosition
	GetWorldPosition(ChunkPosition chunkPos)
	{
		return GetWorldPosition(chunkPos.chunkX, chunkPos.chunkY,
								chunkPos.tileInChunkX, chunkPos.tileInChunkY);
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
	CenteredTilePoint(i32 tileX, i32 tileY)
	{
		WorldPosition result = {};
		result.tileX = tileX;
		result.tileY = tileY;
		return result;
	}
	
	// TODO: Get rid of these. They are used in camera movement code.
	v2
	WorldPosDiff(World* world, WorldPosition* a, WorldPosition* b)
	{
		v2 result;
		// NOTE: Potential integer overflow
		i32 dX = a->tileX - b->tileX;
		i32 dY = a->tileY - b->tileY;
		f32 dOffX = a->offset.x - b->offset.x;
		f32 dOffY = a->offset.y - b->offset.y;
		result = V2(world->tileSizeInUnits * dX + dOffX,
					world->tileSizeInUnits * dY + dOffY);
		return result;
	}

	WorldPosition
	OffsetWorldPos(World* world, WorldPosition pos, v2 offset)
	{
		pos.offset += offset;
		auto newPos = RecanonicalizePosition(world, pos);
		return newPos;
	}

}
