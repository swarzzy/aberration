#pragma once
#include "../shared/Shared.h"
#include "../shared/OpenGL.h"

namespace AB
{
	const u32 CHUNK_MESHER_MAX_CHUNKS = 9;
	struct ChunkMesher
	{
		u32 chunkCount;
		GLuint vertexBuffers[CHUNK_MESHER_MAX_CHUNKS];
		u32 vertexCounts[CHUNK_MESHER_MAX_CHUNKS];
		Chunk* chunks[CHUNK_MESHER_MAX_CHUNKS];
	};

	struct ChunkMesh
	{
		u32 vertexCount;
		u32 blockCount;
		ChunkMeshVertexBlock* head;
		ChunkMeshVertexBlock* tail;	
	};

	void InitializeMesher(ChunkMesher* mesher);
	bool MesherAddChunk(ChunkMesher* mesher, Chunk* chunk);
	void MesherRemoveChunk(ChunkMesher* mesher, Chunk* chunk);
	void MesherUpdateChunks(ChunkMesher* mesher, World* world,
							MemoryArena* tempArena);
	void MesherDrawChunks(ChunkMesher* mesher, Camera* camera,
						  RenderGroup* renderGroup,	AssetManager* assetManager);
	void PushChunkMeshVertex(ChunkMesh* mesh, MemoryArena* arena,
							 v3 position, v3 normal, v2 uv);

	inline void PushChunkMeshQuad(ChunkMesh* mesh, MemoryArena* arena,
								  v3 vtx0, v3 vtx1, v3 vtx2, v3 vtx3);
	inline void PushChunkMeshCube(ChunkMesh* mesh, MemoryArena* arena,
								  v3 min, v3 max);
	ChunkMesh MakeChunkMesh(World* world, Chunk* chunk, MemoryArena* tempArena);
	void UploadChunkMeshToGPU(GLuint vertexBuffer, ChunkMesh* mesh);
	



}
