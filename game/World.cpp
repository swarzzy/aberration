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
	ChangeWorldPosition(World* world, WorldPosition oldPos, v2 offset)
	{
		// TODO: Better checking!!!!!!! CHECKINNNNN!!! ASSERTSS AAA!!
		WorldPosition newPos = oldPos;
		newPos.offset += offset;
		i32 chunkOffsetX = RoundF32I32(newPos.offset.x / world->chunkSizeUnits);
		i32 chunkOffsetY = RoundF32I32(newPos.offset.y / world->chunkSizeUnits);
		newPos.offset.x -= chunkOffsetX * world->chunkSizeUnits;
		newPos.offset.y -= chunkOffsetY * world->chunkSizeUnits;

		AB_ASSERT(oldPos.chunkX + chunkOffsetX > INT32_MIN + CHUNK_SAFE_MARGIN);
		AB_ASSERT(oldPos.chunkX + chunkOffsetX < INT32_MAX - CHUNK_SAFE_MARGIN);

		AB_ASSERT(oldPos.chunkY + chunkOffsetY > INT32_MIN + CHUNK_SAFE_MARGIN);
		AB_ASSERT(oldPos.chunkY + chunkOffsetY < INT32_MAX - CHUNK_SAFE_MARGIN);

		newPos.chunkX += chunkOffsetX;
		newPos.chunkY += chunkOffsetY;

		return newPos;
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
}
