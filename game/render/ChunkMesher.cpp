#include "ChunkMesher.h"

#include "../../shared/OpenGL.h"
namespace AB
{
	void InitializeMesher(ChunkMesher* mesher)
	{
		GLCall(glCreateBuffers(CHUNK_MESHER_MAX_CHUNKS, (GLuint*)(&mesher->vertexBuffers)));
		
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
					mesher->quadCounts[i] = mesher->quadCounts[mesher->chunkCount - 1];
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
		for (u32 i = 0; i < mesher->chunkCount; i++)
		{
			Chunk* chunk = mesher->chunks[i];
			GLuint handle = mesher->vertexBuffers[i];
			if (chunk->dirty)
			{
				//BeginTemporaryMemory(tempArena);
				ChunkMesh mesh = MakeChunkMesh(world, chunk, tempArena);
				mesher->quadCounts[i] = mesh.quadCount;
				UploadChunkMeshToGPU(handle, &mesh);
				AB_INFO("Chunk mesh generated for chunk (X: %i32; Y %i32; Z: %i32).\n Quads: %i32; Vertices: % i32",
					chunk->coordX, chunk->coordY, chunk->coordZ, mesh.quadCount, mesh.quadCount * 4);
				chunk->dirty = false;
				//EndTemporaryMemory(tempArena);
			}
		}
	}

	void MesherDrawChunks(ChunkMesher* mesher, World* world, Camera* camera,
						  RenderGroup* renderGroup,	AssetManager* assetManager)
	{
		for (u32 i = 0; i < mesher->chunkCount; i++)
		{
			Chunk* chunk = mesher->chunks[i];
			GLuint vertexBufferHandle = mesher->vertexBuffers[i];
			u32 quadCount = mesher->quadCounts[i];
			WorldPosition chunkP = {};
			chunkP.tile.x = chunk->coordX * WORLD_CHUNK_DIM_TILES;
			chunkP.tile.y = chunk->coordY * WORLD_CHUNK_DIM_TILES;
			chunkP.tile.z = chunk->coordZ * WORLD_CHUNK_DIM_TILES;
			v3 chunkCamRelP = GetRelativePos(&camera->targetWorldPos, &chunkP);
			
			chunkCamRelP = FlipYZ(chunkCamRelP);
			RenderCommandDrawChunk chunkCommand = {};
			chunkCommand.vboHandle = vertexBufferHandle;
			chunkCommand.quadCount = quadCount;
			chunkCommand.worldMatrix = Translation(chunkCamRelP);
			RenderGroupPushCommand(renderGroup,
								   assetManager,
								   RENDER_COMMAND_DRAW_CHUNK,
								   &chunkCommand);
			
		}
	}

	inline void PushChunkMeshVertex(ChunkMesh* mesh, MemoryArena* arena,
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
	}

	inline void PushChunkMeshQuad(ChunkMesh* mesh, MemoryArena* arena,
								  v3 vtx0, v3 vtx1, v3 vtx2, v3 vtx3,
								  TerrainType type)
	{
		v3 normal = Cross(vtx3 - vtx0, vtx1 - vtx0);
		PushChunkMeshVertex(mesh, arena, vtx0, normal, type);
		PushChunkMeshVertex(mesh, arena, vtx1, normal, type);
		PushChunkMeshVertex(mesh, arena, vtx2, normal, type);		
		PushChunkMeshVertex(mesh, arena, vtx3, normal, type);
		mesh->quadCount++;
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
					u32 tileXMinusOne = tileX ? tileX - 1 : INVALID_TILE_COORD;
					u32 tileYMinusOne = tileY ? tileY - 1 : INVALID_TILE_COORD;
					u32 tileZMinusOne = tileZ ? tileZ - 1 : INVALID_TILE_COORD;
					u32 tileXPlusOne = (tileX < (WORLD_CHUNK_DIM_TILES - 1)) ? tileX + 1 : INVALID_TILE_COORD;
					u32 tileYPlusOne = (tileY < (WORLD_CHUNK_DIM_TILES - 1)) ? tileY + 1 : INVALID_TILE_COORD;
					u32 tileZPlusOne = (tileZ < (WORLD_CHUNK_DIM_TILES - 1)) ? tileZ + 1 : INVALID_TILE_COORD;
					TerrainTileData testTile =
						GetTerrainTile(chunk, V3U(tileX, tileY, tileZ));
					TerrainTileData upTile =
						GetTerrainTile(chunk, V3U(tileX, tileY, tileZPlusOne));
					TerrainTileData dnTile =
						GetTerrainTile(chunk, V3U(tileX, tileY, tileZMinusOne));
					TerrainTileData lTile =
						GetTerrainTile(chunk, V3U(tileXMinusOne, tileY, tileZ));
					TerrainTileData rTile =
						GetTerrainTile(chunk, V3U(tileXPlusOne, tileY, tileZ));
					TerrainTileData fTile =
						GetTerrainTile(chunk, V3U(tileX, tileYPlusOne, tileZ));
					TerrainTileData bTile =
						GetTerrainTile(chunk, V3U(tileX, tileYMinusOne, tileZ));

					if (testTile.type)
					{
						v3 offset = V3(tileX * WORLD_TILE_SIZE,
									   tileZ * WORLD_TILE_SIZE,
									   tileY * WORLD_TILE_SIZE);
						//offset.y -= (WORLD_CHUNK_DIM_TILES) * WORLD_TILE_SIZE;
						v3 min = offset;
						v3 max = offset + V3(WORLD_TILE_SIZE);

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
				}
			}
		}
		return mesh;
	}

	void UploadChunkMeshToGPU(GLuint vertexBuffer, ChunkMesh* mesh)
	{
		GLuint handle = vertexBuffer;
		uptr bufferSize = mesh->quadCount * 4 * (sizeof(v3) + sizeof(v3) + sizeof(byte));
		GLCall(glNamedBufferData(handle, bufferSize, 0, GL_STATIC_DRAW));
#pragma pack(push, 1)
		struct Vertex
		{
			v3 pos;
			v3 normal;
			byte tileId;
		};
#pragma pack(pop)
		Vertex* buffer;
		GLCall(buffer = (Vertex*)glMapNamedBuffer(handle, GL_READ_WRITE));
		AB_ASSERT(buffer);
		u32 bufferCount = 0;
		u32 blockCount = 0;
		ChunkMeshVertexBlock* block = mesh->tail;
		if (block)
		{
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
		}
		
		GLCall(glUnmapNamedBuffer(handle));
	}


}
