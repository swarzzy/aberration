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
		i32 chunkOffsetX = Floor(newPos.offset.x / world->chunkSizeUnits);
		i32 chunkOffsetY = Floor(newPos.offset.y / world->chunkSizeUnits);
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

	// TODO @Important: Potential memory leak here.
	// For now there are no way to delete empty blocks, so
	// entity movement may cause memory leaks
	void
	ChangeEntityPosition(World* world, Entity entity, WorldPosition newPos,
						 MemoryArena* arena)
	{
		WorldPosition oldPos = entity.low->worldPos;
		if(oldPos.chunkX == newPos.chunkX && oldPos.chunkY == newPos.chunkY)
		{
			// NOTE: In same chunk
		}
		else
		{
			// NOTE: Chunk transition
			// Here where fun begins!!!
			{
				Chunk* oldChunk = GetChunk(world, oldPos.chunkX, oldPos.chunkY);
				AB_ASSERT(oldChunk);
				EntityBlock* oldBlock = &oldChunk->firstEntityBlock;
				bool found = false;
				do
				{
					for (u32 i = 0; i < oldBlock->count; i++)
					{
						if (oldBlock->lowEntityIndices[i] == entity.lowIndex)
						{
							if (i != oldBlock->count - 1)
							{
								oldBlock->lowEntityIndices[i] =
									oldBlock->lowEntityIndices[oldBlock->count - 1];
							}
							oldBlock->count--;
							found = true;
							break;
						}
					}
					oldBlock = oldBlock->nextBlock;
				} while (oldBlock && !found);
				AB_ASSERT(found);
			}
			{
				Chunk* newChunk = GetChunk(world, newPos.chunkX, newPos.chunkY);
				AB_ASSERT(newChunk);
				EntityBlock* newBlock = &newChunk->firstEntityBlock;
				bool inserted = false;
				EntityBlock* prevBlock = nullptr;
				do
				{
					if (newBlock->count < ENTITY_BLOCK_CAPACITY)
					{
						newBlock->lowEntityIndices[newBlock->count] =
							entity.lowIndex;
						newBlock->count++;
						inserted = true;
						break;
					}
					prevBlock = newBlock;
					newBlock = newBlock->nextBlock;
				} while (newBlock);

				if (!inserted)
				{
					prevBlock->nextBlock =
						(EntityBlock*)PushSize(arena,
											   sizeof(EntityBlock),
											   alignof(EntityBlock));
					SetZeroScalar(EntityBlock, prevBlock->nextBlock);
					prevBlock->nextBlock->count++;
					prevBlock->nextBlock->lowEntityIndices[0] = entity.lowIndex;
				}
			}
		}
		entity.low->worldPos = newPos;
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

	u32
	AddLowEntity(GameState* gameState, Chunk* chunk, EntityType type,
				 MemoryArena* arena = nullptr)
	{
		AB_ASSERT(gameState->lowEntityCount < MAX_LOW_ENTITIES);
		u32 index = 0;
		if (chunk->firstEntityBlock.count < ENTITY_BLOCK_CAPACITY)
		{
			gameState->lowEntityCount++;
			index = gameState->lowEntityCount;
			gameState->lowEntities[index] = {};
			gameState->lowEntities[index].type = type;
		
			chunk->firstEntityBlock.lowEntityIndices[chunk->firstEntityBlock.count]
				= index;
			chunk->firstEntityBlock.count++;
		}
		else
		{
			EntityBlock* currentBlock = &chunk->firstEntityBlock;
			while (currentBlock->nextBlock)
			{
				currentBlock = currentBlock->nextBlock;
				if (currentBlock->count < ENTITY_BLOCK_CAPACITY)
				{
					gameState->lowEntityCount++;
					index = gameState->lowEntityCount;
					gameState->lowEntities[index] = {};
					gameState->lowEntities[index].type = type;
		
					currentBlock->lowEntityIndices[currentBlock->count]	= index;
					currentBlock->count++;
					break;
				}
			}			
			if ((!index) && arena)
			{
				currentBlock->nextBlock =
					(EntityBlock*)PushSize(arena, sizeof(EntityBlock),
										   alignof(EntityBlock));
				SetZeroScalar(EntityBlock, currentBlock->nextBlock);
				currentBlock = currentBlock->nextBlock;

				gameState->lowEntityCount++;
				index = gameState->lowEntityCount;
				gameState->lowEntities[index] = {};
				gameState->lowEntities[index].type = type;
		
				currentBlock->lowEntityIndices[currentBlock->count]	= index;
				currentBlock->count++;				
			}
		}

		return index;
	}

}
