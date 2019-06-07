#include "Tilemap.h"

namespace AB
{
	inline void RecanonicalizeCoord(f32 tileSizeUnits,
									u32* restrict tileCoord,
									f32* restrict coord)
	{
		i32 tileOffset = RoundF32I32(*coord / tileSizeUnits);
		// TODO: Made this function branchless.
		if (tileOffset < 0)
		{
			if ((AbsI32(tileOffset) > *tileCoord))
			{
				*tileCoord = 0;
				*coord = -0.5f * tileSizeUnits;
			}
			else
			{
				*tileCoord += tileOffset;
				*coord -= (f32)tileOffset * tileSizeUnits;				
			}
		}
		else 
		{
			// NOTE: If u32 has an owerflow, then sum of the coord would
			// be less than the coord itsef. It will be wrong if
			// an overflowed sum will be equal to the coord
			if (tileOffset + *tileCoord < (u32)(*tileCoord))
			{
				*tileCoord = 0xffffffff;
				*coord = 0.5f * tileSizeUnits;
			}
			else
			{
				*tileCoord += tileOffset;
				*coord -= (f32)tileOffset * tileSizeUnits;				
			}
		}
	}

	inline TilemapPosition
	MapToTileSpace(Tilemap* tilemap, TilemapPosition worldCenter, v2 relPos)
	{
		TilemapPosition result = worldCenter;
		result.offset += relPos;
		RecanonicalizeCoord(tilemap->tileSizeInUnits, &result.tileX,
							&result.offset.x);
		RecanonicalizeCoord(tilemap->tileSizeInUnits, &result.tileY,
							&result.offset.y);

		AB_ASSERT(result.offset.x >= -tilemap->tileRadiusInUnits);
		AB_ASSERT(result.offset.y >= -tilemap->tileRadiusInUnits);
		AB_ASSERT(result.offset.x <= tilemap->tileRadiusInUnits);
		AB_ASSERT(result.offset.y <= tilemap->tileRadiusInUnits);

		return result;
	}

	inline TilemapPosition RecanonicalizePosition(const Tilemap* tilemap,
												  TilemapPosition pos)
	{
		RecanonicalizeCoord(tilemap->tileSizeInUnits, &pos.tileX, &pos.offset.x);
		RecanonicalizeCoord(tilemap->tileSizeInUnits, &pos.tileY, &pos.offset.y);

		AB_ASSERT(pos.offset.x >= -tilemap->tileRadiusInUnits);
		AB_ASSERT(pos.offset.y >= -tilemap->tileRadiusInUnits);
		AB_ASSERT(pos.offset.x <= tilemap->tileRadiusInUnits);
		AB_ASSERT(pos.offset.y <= tilemap->tileRadiusInUnits);

		return pos;
	}

	inline Chunk*
	GetChunk(Tilemap* tilemap, u32 chunkX, u32 chunkY,
			 MemoryArena* arena = nullptr)
	{
		Chunk* result = nullptr;
		// TODO: Check is chunk coord are valid
		u32 chunkHash = (chunkX * 22 + chunkY * 12) % CHUNK_TABLE_SIZE;
		AB_ASSERT(chunkHash < CHUNK_TABLE_SIZE);

		Chunk* currentChunk = tilemap->chunkTable + chunkHash;
		if (currentChunk->coord.x != INVALID_CHUNK_COORD)
		{
			if (currentChunk->coord.x = chunkX && currentChunk->coord.y)
			{
				result = currentChunk;
			}
			else
			{
				while (currentChunk->nextChunk)
				{
					Chunk* nextChunk = currentChunk->nextChunk;
					if (nextChunk->coord.x = chunkX && nextChunk->coord.y)
					{
						result = nextChunk;
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
			if (currentChunk->coord.x == INVALID_CHUNK_COORD)
			{
				newChunk = currentChunk;
			}
			else
			{
				newChunk = (Chunk*)PushSize(arena, sizeof(Chunk),
												   alignof(Chunk));
				currentChunk->nextChunk = newChunk;
			}

			// TODO: Store tiles directly in chunks
			uptr tileMemSize = sizeof(u32)
				* tilemap->chunkSizeInTiles * tilemap->chunkSizeInTiles;
			uptr colorMemSize = sizeof(v3)
				* tilemap->chunkSizeInTiles * tilemap->chunkSizeInTiles;
			newChunk->tiles = (u32*)PushSize(arena, tileMemSize, 0);
			newChunk->colors = (v3*)PushSize(arena, colorMemSize, 0);
			newChunk->coord.x = chunkX;
			newChunk->coord.y = chunkY;
			newChunk->nextChunk = nullptr;

			result = newChunk;
		}
		
		return result;
	}

	inline ChunkPosition GetChunkPosition(const Tilemap* tilemap, u32 absTileX,
										  u32 absTileY)
	{
		ChunkPosition result;
		result.chunkX = absTileX >> tilemap->chunkShift;
		result.chunkY = absTileY >> tilemap->chunkShift;
		result.tileInChunkX = absTileX & tilemap->chunkMask;
		result.tileInChunkY = absTileY & tilemap->chunkMask;

		return result;
	}

	inline TilemapPosition
	GetTilemapPosition(Tilemap* tilemap, u32 chunkX, u32 chunkY,
					   u32 relTileX, u32 relTileY)
	{
		TilemapPosition result = {};
		result.tileX = chunkX * tilemap->chunkSizeInTiles;
		result.tileY = chunkY * tilemap->chunkSizeInTiles;
		result.tileX += relTileX;
		result.tileY += relTileY;

		return result;		
	}

	inline TilemapPosition
	GetTilemapPosition(Tilemap* tilemap, ChunkPosition chunkPos)
	{
		return GetTilemapPosition(tilemap, chunkPos.chunkX, chunkPos.chunkY,
								  chunkPos.tileInChunkX, chunkPos.tileInChunkY);
	}

	
	inline u32 GetTileValueInChunk(const Tilemap* tilemap, const Chunk* chunk,
							u32 tileX, u32 tileY)
	{
		u32 result = 0;
		
		AB_ASSERT(tileX < tilemap->chunkSizeInTiles);
		AB_ASSERT(tileY < tilemap->chunkSizeInTiles);
		
		if (chunk && chunk->tiles)
		{
			result = chunk->tiles[tileY * tilemap->chunkSizeInTiles + tileX];
		}
		
		return result;
	}

	inline u32 GetTileValue(Tilemap* tilemap, u32 absTileX, u32 absTileY)
	{
		u32 result = 0;
		ChunkPosition chunkPos = GetChunkPosition(tilemap, absTileX, absTileY);
		Chunk* chunk = GetChunk(tilemap, chunkPos.chunkX, chunkPos.chunkY);
		if (chunk)
		{
			result = GetTileValueInChunk(tilemap, chunk,
										 chunkPos.tileInChunkX,
										 chunkPos.tileInChunkY);
		}
		return result;		
	}

	inline void SetTileValueInChunk(MemoryArena* arena, const Tilemap* tilemap,
									Chunk* chunk, u32 tileX, u32 tileY, u32 value)
	{
		AB_ASSERT(tileX < tilemap->chunkSizeInTiles);
		AB_ASSERT(tileY < tilemap->chunkSizeInTiles);
		if (chunk->tiles == nullptr)
		{
			chunk->tiles = (u32*)PushSize(arena,
										  tilemap->chunkSizeInTiles *
										  tilemap->chunkSizeInTiles *
										  sizeof(u32),
										  0);
			AB_ASSERT(chunk->tiles);

			// TODO: Allocating colors temporary
			chunk->colors = (v3*)PushSize(arena,
										  tilemap->chunkSizeInTiles *
										  tilemap->chunkSizeInTiles *
										  sizeof(v3),
										  0);
			AB_ASSERT(chunk->tiles);
		}

		chunk->tiles[tileY * tilemap->chunkSizeInTiles + tileX] = value;		
	}

	inline void SetTileValue(MemoryArena* arena, Tilemap* tilemap,
							 u32 absTileX, u32 absTileY, u32 value)
	{
		ChunkPosition cPos = GetChunkPosition(tilemap, absTileX, absTileY);
		Chunk* chunk = GetChunk(tilemap, cPos.chunkX, cPos.chunkY);
		AB_ASSERT(chunk, "Chunk was nullptr");
		SetTileValueInChunk(arena, tilemap, chunk,
							cPos.tileInChunkX, cPos.tileInChunkY, value);
	}

	b32 TestWorldPoint(Tilemap* tilemap, TilemapPosition p)
	{
		u32 tileValue = GetTileValue(tilemap, p.tileX, p.tileY);
		return tileValue == 1;
	}

	inline TilemapPosition CenteredTilePoint(u32 tileX, u32 tileY)
	{
		TilemapPosition result = {};
		result.tileX = tileX;
		result.tileY = tileY;
		return result;
	}

	v2 TilemapPosDiff(const Tilemap* tilemap, const TilemapPosition* a, const TilemapPosition* b)
	{
		v2 result;
		// NOTE: Potential integer overflow
		i32 dX = a->tileX - b->tileX;
		i32 dY = a->tileY - b->tileY;
		f32 dOffX = a->offset.x - b->offset.x;
		f32 dOffY = a->offset.y - b->offset.y;
		result = V2(tilemap->tileSizeInUnits * dX + dOffX,
					tilemap->tileSizeInUnits * dY + dOffY);
		return result;
	}

	TilemapPosition OffsetTilemapPos(const Tilemap* tilemap,
									 TilemapPosition pos, v2 offset)
	{
		pos.offset += offset;
		auto newPos = RecanonicalizePosition(tilemap, pos);
		return newPos;
	}

	// NOTE: Temporary func just for test
	inline void SetTileColor(Tilemap* tilemap, Chunk* chunk,
							 u32 tileX, u32 tileY, v3 color)
	{
		AB_ASSERT(chunk, "Chunk was nullptr");
		AB_ASSERT(tileX < tilemap->chunkSizeInTiles);
		AB_ASSERT(tileY < tilemap->chunkSizeInTiles);

		chunk->colors[tileY * tilemap->chunkSizeInTiles + tileX] = color;		
	}

	inline v3 GetTileColor(const Tilemap* tilemap, Chunk* chunk,
							u32 tileX, u32 tileY)
	{
		v3 result = {};
		
		AB_ASSERT(tileX < tilemap->chunkSizeInTiles);
		AB_ASSERT(tileY < tilemap->chunkSizeInTiles);
		
		if (chunk && chunk->tiles)
		{
			result = chunk->colors[tileY * tilemap->chunkSizeInTiles + tileX];
		}
		
		return result;
	}


}
