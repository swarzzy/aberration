#include "ChunkMesher.h"

#include "../../shared/OpenGL.h"
namespace AB
{
	void InitializeMesher(ChunkMesher* mesher)
	{
		GLCall(glGenBuffers(CHUNK_MESHER_MAX_CHUNKS, (GLuint*)(&mesher->vertexBuffers)));
		
#if defined (AB_CONFIG_DEBUG)
		for (u32 i = 0; i < CHUNK_MESHER_MAX_CHUNKS; i++)
		{
			AB_ASSERT(mesher->vertexBuffers[i]);
		}
#endif
	}

	bool MesherAddChunk(ChunkMesher* mesher, Chunk* chunk)
	{
		bool result = false;
		if (mesher->chunkCount < CHUNK_MESHER_MAX_CHUNKS)
		{
			mesher->chunks[mesher->chunkCount] = chunk;
			mesher->chunkCount++;
			result = true;
			chunk->dirty = true;
		}
		return result;
	}

	void MesherRemoveChunk(ChunkMesher* mesher, Chunk* chunk)
	{
		for (u32 i = 0; i < mesher->chunkCount; i++)
		{
			if (chunk == mesher->chunks[i])
			{
				GLuint vbHandle = mesher->vertexBuffers[i];

				if (i == mesher->chunkCount - 1)
				{
					mesher->chunkCount--;
				}
				else
				{
					mesher->chunks[i] = mesher->chunks[mesher->chunkCount - 1];
					// TODO: We can loose vbo handle this way
					// Store handles in another array and one from there when chunk added
					
					mesher->vertexBuffers[i] =
						mesher->vertexBuffers[mesher->chunkCount - 1];
					mesher->vertexBuffers[mesher->chunkCount - 1] = vbHandle;
					mesher->vertexCounts[i] = mesher->vertexCounts[mesher->chunkCount - 1];
					mesher->chunkCount--;
				}
				chunk->dirty = true;
				GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbHandle));
				GLCall(glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW));
				break;
			}
		}
	}

	void MesherUpdateChunks(ChunkMesher* mesher, World* world,
							MemoryArena* tempArena)
	{
		BeginTemporaryMemory(tempArena);
		for (u32 i = 0; i < mesher->chunkCount; i++)
		{
			Chunk* chunk = mesher->chunks[i];
			GLuint handle = mesher->vertexBuffers[i];
			if (chunk->dirty)
			{
				ChunkMesh mesh = MakeChunkMesh(world, chunk, tempArena);
				mesher->vertexCounts[i] = mesh.vertexCount;
				UploadChunkMeshToGPU(handle, &mesh);
				chunk->dirty = false;
			}
		}
		EndTemporaryMemory(tempArena);
	}

	void MesherDrawChunks(ChunkMesher* mesher, World* world, Camera* camera,
						  RenderGroup* renderGroup,	AssetManager* assetManager)
	{
		for (u32 i = 0; i < mesher->chunkCount; i++)
		{
			Chunk* chunk = mesher->chunks[i];
			GLuint vertexBufferHandle = mesher->vertexBuffers[i];
			u32 vertexCount = mesher->vertexCounts[i];
			WorldPosition chunkP = {};
			chunkP.chunkX = chunk->coordX;
			chunkP.chunkY = chunk->coordY;
			v3 chunkCamRelP = GetCamRelPos(world, chunkP,
										   camera->targetWorldPos);
			
			chunkCamRelP = FlipYZ(chunkCamRelP);
			RenderCommandDrawChunk chunkCommand = {};
			chunkCommand.vboHandle = vertexBufferHandle;
			chunkCommand.numVertices = vertexCount;
			chunkCommand.worldMatrix = Translation(chunkCamRelP);
			RenderGroupPushCommand(renderGroup,
								   assetManager,
								   RENDER_COMMAND_DRAW_CHUNK,
								   &chunkCommand);
			
		}
	}

	void PushChunkMeshVertex(ChunkMesh* mesh, MemoryArena* arena,
							 v3 position, v3 normal, byte tileId)
	{
		if (!mesh->head)
		{
			mesh->head =
				(ChunkMeshVertexBlock*)PushSize(arena, sizeof(ChunkMeshVertexBlock), 0);
			SetZeroScalar(ChunkMeshVertexBlock, mesh->head);
			mesh->tail = mesh->head;
			AB_ASSERT(mesh->head);
			mesh->blockCount++;
		}
		else if (mesh->head->at >= CHUNK_MESH_MEM_BLOCK_CAPACITY)
		{
			ChunkMeshVertexBlock* newBlock =
				(ChunkMeshVertexBlock*)PushSize(arena, sizeof(ChunkMeshVertexBlock), 0);
			AB_ASSERT(newBlock);
			// TODO: Set to zero?
			SetZeroScalar(ChunkMeshVertexBlock, newBlock);
			mesh->head->prevBlock = newBlock;
			newBlock->nextBlock = mesh->head;
			
			mesh->head = newBlock;
			mesh->blockCount++;
		}
		
		mesh->head->positions[mesh->head->at] = position;
		mesh->head->normals[mesh->head->at] = normal;
		mesh->head->tileIds[mesh->head->at] = tileId;
		mesh->head->at++;
		mesh->vertexCount++;
	}

	inline void PushChunkMeshQuad(ChunkMesh* mesh, MemoryArena* arena,
								  v3 vtx0, v3 vtx1, v3 vtx2, v3 vtx3,
								  TerrainType type)
	{
		v3 normal = Cross(vtx3 - vtx0, vtx1 - vtx0);
		PushChunkMeshVertex(mesh, arena, vtx2, normal, type - 1);
		PushChunkMeshVertex(mesh, arena, vtx1, normal, type - 1);
		PushChunkMeshVertex(mesh, arena, vtx0, normal, type - 1);
		
		PushChunkMeshVertex(mesh, arena, vtx0, normal, type - 1);
		PushChunkMeshVertex(mesh, arena, vtx3, normal, type - 1);
		PushChunkMeshVertex(mesh, arena, vtx2, normal, type - 1);
	}

	inline void PushChunkMeshCube(ChunkMesh* mesh, MemoryArena* arena,
								  v3 min, v3 max, TerrainType type)
	{
		v3 vtx0 = min;
		v3 vtx1 = V3(max.x, min.y, min.z);
		v3 vtx2 = V3(max.x, min.y, max.z);
		v3 vtx3 = V3(min.x, min.y, max.z);

		v3 vtx4 = V3(min.x, max.y, min.z);
		v3 vtx5 = V3(max.x, max.y, min.z);
		v3 vtx6 = V3(max.x, max.y, max.z);
		v3 vtx7 = V3(min.x, max.y, max.z);

		PushChunkMeshQuad(mesh, arena, vtx0, vtx1, vtx5, vtx4, type);
		PushChunkMeshQuad(mesh, arena, vtx1, vtx2, vtx6, vtx5, type);
		PushChunkMeshQuad(mesh, arena, vtx0, vtx3, vtx2, vtx1, type);
		PushChunkMeshQuad(mesh, arena, vtx5, vtx6, vtx7, vtx4, type);
		PushChunkMeshQuad(mesh, arena, vtx3, vtx0, vtx4, vtx7, type);
		PushChunkMeshQuad(mesh, arena, vtx2, vtx3, vtx7, vtx6, type);
	}


	ChunkMesh MakeChunkMesh(World* world, Chunk* chunk, MemoryArena* tempArena)
	{
		ChunkMesh mesh = {};
		for (u32 tileZ = 0; tileZ < WORLD_CHUNK_DIM_TILES; tileZ++)
		{
			for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
			{
				for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
				{
#if 1
					u32 tileXMinusOne = tileX ? tileX - 1 : INVALID_TILE_COORD;
					u32 tileYMinusOne = tileY ? tileY - 1 : INVALID_TILE_COORD;
					u32 tileZMinusOne = tileZ ? tileZ - 1 : INVALID_TILE_COORD;
					u32 tileXPlusOne = (tileX < (WORLD_CHUNK_DIM_TILES - 1)) ? tileX + 1 : INVALID_TILE_COORD;
					u32 tileYPlusOne = (tileY < (WORLD_CHUNK_DIM_TILES - 1)) ? tileY + 1 : INVALID_TILE_COORD;
					u32 tileZPlusOne = (tileZ < (WORLD_CHUNK_DIM_TILES - 1)) ? tileZ + 1 : INVALID_TILE_COORD;
					TerrainTileData testTile =
						GetTerrainTile(chunk, tileX, tileY, tileZ);
					TerrainTileData upTile =
						GetTerrainTile(chunk, tileX, tileY, tileZPlusOne);
					TerrainTileData dnTile =
						GetTerrainTile(chunk, tileX, tileY, tileZMinusOne);
					TerrainTileData lTile =
						GetTerrainTile(chunk, tileXMinusOne, tileY, tileZ);
					TerrainTileData rTile =
						GetTerrainTile(chunk, tileXPlusOne, tileY, tileZ);
					TerrainTileData fTile =
						GetTerrainTile(chunk, tileX, tileYPlusOne, tileZ);
					TerrainTileData bTile =
						GetTerrainTile(chunk, tileX, tileYMinusOne, tileZ);

					if (testTile.type)
					{
						v3 offset = V3(tileX * world->tileSizeInUnits + world->tileRadiusInUnits,
									   tileZ * world->tileSizeInUnits + world->tileRadiusInUnits,
									   tileY * world->tileSizeInUnits + world->tileRadiusInUnits);
						offset.y -= (WORLD_CHUNK_DIM_TILES) * world->tileSizeInUnits;
						v3 min = offset - V3(world->tileRadiusInUnits);
						v3 max = offset + V3(world->tileRadiusInUnits);
						min *= world->unitsToRaw;
						max *= world->unitsToRaw;

						TerrainType type = testTile.type;
						v3 vtx0 = min;
						v3 vtx1 = V3(max.x, min.y, min.z);
						v3 vtx2 = V3(max.x, min.y, max.z);
						v3 vtx3 = V3(min.x, min.y, max.z);

						v3 vtx4 = V3(min.x, max.y, min.z);
						v3 vtx5 = V3(max.x, max.y, min.z);
						v3 vtx6 = V3(max.x, max.y, max.z);
						v3 vtx7 = V3(min.x, max.y, max.z);

						if (!NotEmpty(&upTile))
						{
							PushChunkMeshQuad(&mesh, tempArena, vtx5, vtx6, vtx7, vtx4, type);
						}
						if (!NotEmpty(&dnTile))
						{
							PushChunkMeshQuad(&mesh, tempArena, vtx0, vtx3, vtx2, vtx1, type);
						}
						if (!NotEmpty(&rTile))
						{
							PushChunkMeshQuad(&mesh, tempArena, vtx1, vtx2, vtx6, vtx5, type);
						}
						if (!NotEmpty(&lTile))
						{
							PushChunkMeshQuad(&mesh, tempArena, vtx3, vtx0, vtx4, vtx7, type);
						}
						if (!NotEmpty(&fTile))
						{
							PushChunkMeshQuad(&mesh, tempArena, vtx2, vtx3, vtx7, vtx6, type);
						}
						if (!NotEmpty(&bTile))
						{
							PushChunkMeshQuad(&mesh, tempArena, vtx0, vtx1, vtx5, vtx4, type);
						}
					}
#else
					u32 tileXMinusOne = tileX ? tileX - 1 : INVALID_TILE_COORD;
					u32 tileYMinusOne = tileY ? tileY - 1 : INVALID_TILE_COORD;
					u32 tileZMinusOne = tileZ ? tileZ - 1 : INVALID_TILE_COORD;
					u32 tileXPlusOne = tileX < (WORLD_CHUNK_DIM_TILES - 1) ? tileX + 1 : INVALID_TILE_COORD;
					u32 tileYPlusOne = tileY < (WORLD_CHUNK_DIM_TILES - 1) ? tileY + 1 : INVALID_TILE_COORD;
					u32 tileZPlusOne = tileZ < (WORLD_CHUNK_DIM_TILES - 1) ? tileZ + 1 : INVALID_TILE_COORD;
					TerrainTileData testTile =
											   GetTerrainTile(chunk, tileX, tileY, tileZ);
					TerrainTileData upTile =
											   GetTerrainTile(chunk, tileX, tileY, tileZPlusOne);
					TerrainTileData dnTile =
											   GetTerrainTile(chunk, tileX, tileY, tileZMinusOne);
					TerrainTileData lTile =
											   GetTerrainTile(chunk, tileXMinusOne, tileY, tileZ);
					TerrainTileData rTile =
											   GetTerrainTile(chunk, tileXPlusOne, tileY, tileZ);
					TerrainTileData fTile =
											   GetTerrainTile(chunk, tileX, tileYPlusOne, tileZ);
					TerrainTileData bTile =
											   GetTerrainTile(chunk, tileX, tileYMinusOne, tileZ);

					bool occluded = NotEmpty(&upTile) &&
						NotEmpty(&dnTile) &&
						NotEmpty(&lTile) &&
						NotEmpty(&rTile) &&
						NotEmpty(&fTile) &&
						NotEmpty(&bTile);
					
					if (!occluded && testTile.type)
					{
						v3 offset = V3(tileX * world->tileSizeInUnits + world->tileRadiusInUnits,
									   tileZ * world->tileSizeInUnits + world->tileRadiusInUnits,
									   tileY * world->tileSizeInUnits + world->tileRadiusInUnits);
						//offset -= (WORLD_CHUNK_DIM_TILES / 2) * world->tileSizeInUnits;
						offset.y -= (WORLD_CHUNK_DIM_TILES) * world->tileSizeInUnits;
						v3 min = offset - V3(world->tileRadiusInUnits );
						v3 max = offset + V3(world->tileRadiusInUnits );
						min *= world->unitsToRaw;
						max *= world->unitsToRaw;
						PushChunkMeshCube(&mesh, tempArena, min, max, testTile.type);
					}
#endif
				}
			}
		}
		return mesh;
	}

	void UploadChunkMeshToGPU(GLuint vertexBuffer, ChunkMesh* mesh)
	{
		GLuint handle = vertexBuffer;
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, handle));
		uptr bufferSize = mesh->vertexCount * (sizeof(v3) + sizeof(v3) + sizeof(byte));
		GLCall(glBufferData(GL_ARRAY_BUFFER, bufferSize, 0, GL_STATIC_DRAW));
		#pragma pack(push, 1)
		struct Vertex
		{
			v3 pos;
			v3 normal;
			byte tileId;
		};
		#pragma pack(pop)
		Vertex* buffer;
		GLCall(buffer = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
		AB_ASSERT(buffer);
		u32 bufferCount = 0;
		u32 blockCount = 0;
		ChunkMeshVertexBlock* block = mesh->tail;
		do
		{
			blockCount++;
			for (u32 i = 0; i < block->at; i++)
			{
				buffer[bufferCount].pos = block->positions[i];
				buffer[bufferCount].normal = block->normals[i];
				buffer[bufferCount].tileId = block->tileIds[i];
				bufferCount++;
			}
			block = block->prevBlock;
		}
		while(block);
		
		GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
	}


}
