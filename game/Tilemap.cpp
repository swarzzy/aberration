#include "Tilemap.h"

namespace AB
{
	inline void RecanonicalizeCoord(f32 tileSizeUnits,
									u32* tileCoord,
									f32* coord)
	{
		i32 tileOffset = RoundF32I32(*coord / tileSizeUnits);
		// NOTE: tileCoord might go out if bounds here
		// It will be clamped in RecanonicalizePosition
		*tileCoord += tileOffset;
		*coord -= (f32)tileOffset * tileSizeUnits;
	}

	inline TilemapPosition RecanonicalizePosition(Tilemap* tilemap,
												  TilemapPosition pos)
	{
		RecanonicalizeCoord(tilemap->tileSizeInUnits, &pos.tileX, &pos.offsetX);
		RecanonicalizeCoord(tilemap->tileSizeInUnits, &pos.tileY, &pos.offsetY);

		AB_ASSERT(pos.offsetX >= -tilemap->tileRadiusInUnits);
		AB_ASSERT(pos.offsetY >= -tilemap->tileRadiusInUnits);
		AB_ASSERT(pos.offsetX < tilemap->tileRadiusInUnits);
		AB_ASSERT(pos.offsetY < tilemap->tileRadiusInUnits);

		return pos;
	}

	inline Chunk* GetChunk(Tilemap* tilemap, u32 chunkX, u32 chunkY)
	{
		Chunk* chunk = nullptr;
		if (chunkX < tilemap->tilemapChunkCountX &&
			chunkY < tilemap->tilemapChunkCountY)
		{
			chunk =  &tilemap->chunks[chunkY * tilemap->tilemapChunkCountX +
									  chunkX];
		}
		return chunk;
	}

	inline ChunkPosition GetChunkPosition(Tilemap* tilemap, u32 absTileX,
										  u32 absTileY)
	{
		ChunkPosition result;
		result.chunkX = absTileX >> tilemap->chunkShift;
		result.chunkY = absTileY >> tilemap->chunkShift;
		result.tileInChunkX = absTileX & tilemap->chunkMask;
		result.tileInChunkY = absTileY & tilemap->chunkMask;

		return result;
	}

	inline u32 GetTileValueInChunk(Tilemap* tilemap, Chunk* chunk,
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

	inline u32 GetTileValue(Tilemap* tilemap, u32 absChunkX, u32 absChunkY)
	{
		u32 result = 0;
		ChunkPosition chunkPos = GetChunkPosition(tilemap, absChunkX, absChunkY);
		Chunk* chunk = GetChunk(tilemap, chunkPos.chunkX, chunkPos.chunkY);
		if (chunk)
		{
			result = GetTileValueInChunk(tilemap, chunk,
										 chunkPos.tileInChunkX,
										 chunkPos.tileInChunkY);
		}
		return result;		
	}

	inline void SetTileValueInChunk(MemoryArena* arena, Tilemap* tilemap,
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

	// NOTE: Temporary func just for test
	inline void SetTileColor(Tilemap* tilemap, Chunk* chunk,
							 u32 tileX, u32 tileY, v3 color)
	{
		AB_ASSERT(chunk, "Chunk was nullptr");
		AB_ASSERT(tileX < tilemap->chunkSizeInTiles);
		AB_ASSERT(tileY < tilemap->chunkSizeInTiles);

		chunk->colors[tileY * tilemap->chunkSizeInTiles + tileX] = color;		
	}

	inline v3 GetTileColor(Tilemap* tilemap, Chunk* chunk,
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
