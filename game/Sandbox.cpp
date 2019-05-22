#include "Sandbox.h"
// TODO: This is temporary for rand()
#include <stdlib.h>

#include "Tilemap.cpp"
#include "Camera.cpp"


namespace AB
{
	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  Sandbox* sandbox,
			  AssetManager* assetManager)
	{
		sandbox->renderGroup = AllocateRenderGroup(arena, MEGABYTES(300),
												   128000, 32);
#if 0
		sandbox->renderGroup->projectionMatrix = PerspectiveRH(45.0f,
															   16.0f / 9.0f,
															   0.1f,
															   300.0f);
#endif
		sandbox->dirLight.ambient = V3(0.05f);
		sandbox->dirLight.diffuse = V3(1.1f);
		sandbox->dirLightOffset = V3(-10, 38, 17);
		
		BeginTemporaryMemory(tempArena);
#if 0
		sandbox->mansionMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/mansion/mansion.aab");
		sandbox->planeMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena, tempArena,
							   "../assets/Plane.aab");
#endif
		
		//AB_CORE_ASSERT(sandbox->mansionMeshHandle != ASSET_INVALID_HANDLE);
		EndTemporaryMemory(tempArena);

		sandbox->camera = {};
		sandbox->camera.longSmoothness = 0.3f;
		sandbox->camera.latSmoothness = 0.3f;
		sandbox->camera.distSmoothness = 0.3f;
		sandbox->camera.pos = V3(0.0f, 0.0f, -1.0f);
		sandbox->camera.front = V3(0.0, 0.0f, 1.0f);
		sandbox->camera.fov = 45.0f;
		sandbox->camera.aspectRatio = 16.0f / 9.0f;
		sandbox->camera.nearPlane = 0.1f;
		sandbox->camera.farPlane = 150.0f;
		sandbox->camera.targetWorldPos.tileX = 16 * 2 + 3; // 
		sandbox->camera.targetWorldPos.tileY = 16 * 2 + 3;
		sandbox->camera.targetWorldPos.offset.x = 1.0f;
		sandbox->camera.targetWorldPos.offset.y = 1.0f;
		sandbox->camera.cullPosAdjust = 1.5f;

	
		//SubscribeKeyboardEvents(&sandbox->camera, inputManager,
		//						&sandbox->inputState);

		sandbox->gamma = 2.2f;
		sandbox->exposure = 1.0f;

		sandbox->world = (World*)PushSize(arena, sizeof(World), alignof(World));
		AB_ASSERT(sandbox->world);

		Tilemap* tilemap = &(sandbox->world->tilemap);

		tilemap->chunkShift = 4;
		tilemap->chunkMask = (1 << tilemap->chunkShift) - 1;
		tilemap->chunkSizeInTiles = (1 << tilemap->chunkShift);
		tilemap->tilemapChunkCountX = 17;
		tilemap->tilemapChunkCountY = 17;
		tilemap->tileSizeRaw = 3.0f;
		tilemap->tileSizeInUnits = 1.0f;
		tilemap->tileRadiusInUnits = 0.5f;
		tilemap->toUnits = tilemap->tileSizeInUnits / tilemap->tileSizeRaw;
		tilemap->unitsToRaw = tilemap->tileSizeRaw / tilemap->tileSizeInUnits;

		tilemap->chunks = (Chunk*)PushSize(arena,
										   tilemap->tilemapChunkCountX *
										   tilemap->tilemapChunkCountY *
										   sizeof(Chunk),
										   0);
		AB_ASSERT(tilemap->chunks);

		f32 r = 0.2f;
		f32 g = 0.2f;
		f32 b = 0.2f;
		
		for (u32 y = 1; y < tilemap->tilemapChunkCountX; y++)
		{
			for (u32 x = 1; x < tilemap->tilemapChunkCountY; x++)
			{
				r = rand() % 11 / 10.0f;
				g = rand() % 11 / 10.0f;
				b = rand() % 11 / 10.0f;
				Chunk* chunk = GetChunk(tilemap, x, y);
				for (u32 tileY = 0; tileY < tilemap->chunkSizeInTiles; tileY++)
				{
					for (u32 tileX = 0; tileX < tilemap->chunkSizeInTiles; tileX++)
					{
						if (tileX == 0 && tileY == 0)
						{
							SetTileValueInChunk(arena, tilemap, chunk,
												tileX, tileY, 3);
								
						}
						else
						{
							SetTileValueInChunk(arena, tilemap, chunk,
												tileX, tileY, 1);
						}
						SetTileColor(tilemap, chunk,
									 tileX, tileY, V3(r, g ,b));

					}
				}
			}
		}
	

	}
		
	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer)
	{
		DEBUG_OVERLAY_SLIDER(g_Platform->gameSpeed, 0.0f, 10.0f);
		Tilemap* tilemap = &sandbox->world->tilemap;
		
		MoveCameraTarget(&sandbox->camera, tilemap);
		ChunkPosition chunkPos =
			GetChunkPosition(tilemap,
							 sandbox->camera.targetWorldPos.tileX,
							 sandbox->camera.targetWorldPos.tileY);

		UpdateCamera(&sandbox->camera,
					 sandbox->renderGroup, tilemap);
		DrawWorldInstancedMinMax(tilemap, sandbox->renderGroup, assetManager,
								 sandbox->camera.targetWorldPos,
								 sandbox->camera.frustumGroundAABB,
								 &sandbox->camera.frustumGroundPoints);

		f32 pX = (chunkPos.tileInChunkX * tilemap->tileSizeInUnits +
				  sandbox->camera.targetWorldPos.offset.x) * tilemap->unitsToRaw;
		f32 pY = (chunkPos.tileInChunkY * tilemap->tileSizeInUnits +
				  sandbox->camera.targetWorldPos.offset.y) * tilemap->unitsToRaw;

		pY = (tilemap->chunkSizeInTiles * tilemap->tileSizeRaw) - pY;
			
		DrawDebugCube(sandbox->renderGroup,
					  assetManager,
					  V3(0.0f, 3.0f, 0.0f),
					  //V3(pX, 3.0f, pY),
					  tilemap->tileSizeRaw * 0.5f, V3(1.0f, 0.0f, 0.0f));

		//sandbox->camera.target = V3(pX, 0, pY);
		//sandbox->dirLight.target = V3(pX, 0, pY);

		DEBUG_OVERLAY_TRACE_VAR(pX);
		DEBUG_OVERLAY_TRACE_VAR(pY);
		DEBUG_OVERLAY_TRACE_VAR(chunkPos.tileInChunkX);
		DEBUG_OVERLAY_TRACE_VAR(chunkPos.tileInChunkY);
		
		DEBUG_OVERLAY_PUSH_SLIDER("Gamma", &sandbox->gamma, 0.0f, 3.0f);
		DEBUG_OVERLAY_PUSH_VAR("Gamma", sandbox->gamma);

		DEBUG_OVERLAY_PUSH_SLIDER("Exposure", &sandbox->exposure, 0.0f, 3.0f);
		DEBUG_OVERLAY_PUSH_VAR("Exposure", sandbox->exposure);

		renderer->cc.gamma = sandbox->gamma;
		renderer->cc.exposure = sandbox->exposure;
		
		RenderCommandDrawMesh planeCommand = {};
		planeCommand.meshHandle = sandbox->mansionMeshHandle;
		planeCommand.transform.worldMatrix = Identity4();
		planeCommand.blendMode = BLEND_MODE_OPAQUE;

		f32 ka = sandbox->dirLight.ambient.r;
		f32 kd = sandbox->dirLight.diffuse.r;
	
		DEBUG_OVERLAY_PUSH_SLIDER("Ka", &ka, 0.0f, 1.0f);
		DEBUG_OVERLAY_PUSH_VAR("Ka", ka);

		DEBUG_OVERLAY_PUSH_SLIDER("Kd", &kd, 0.0f, 10.0f);
		DEBUG_OVERLAY_PUSH_VAR("Kd", kd);

		DEBUG_OVERLAY_TRACE_VAR(sandbox->dirLight.from);
		DEBUG_OVERLAY_TRACE_VAR(sandbox->dirLight.target);
		DEBUG_OVERLAY_TRACE_VAR(sandbox->dirLightOffset);
		DEBUG_OVERLAY_PUSH_SLIDER("offset", &sandbox->dirLightOffset, -50, 50);

		sandbox->dirLight.from = AddV3V3(sandbox->dirLight.target,
										 sandbox->dirLightOffset);
		sandbox->dirLight.ambient = V3(ka);
		sandbox->dirLight.diffuse = V3(kd);
		

		RenderCommandSetDirLight dirLightCommand = {};
		dirLightCommand.light = sandbox->dirLight;

		RenderGroupPushCommand(sandbox->renderGroup,
							   assetManager,
							   RENDER_COMMAND_SET_DIR_LIGHT,
							   (void*)(&dirLightCommand));

		RendererRender(renderer, assetManager, sandbox->renderGroup);
		RenderGroupResetQueue(sandbox->renderGroup);
	}

}


// NOTE: Retired
#if 0
#define TEST
#if !defined(TEST)
void DrawWorld(Tilemap* tilemap, RenderGroup* renderGroup,
			   AssetManager* assetManager, TilemapPosition player)
{
	Chunk* chunks[9];
	SetArray(Chunk*, 9, &chunks, 0);
	ChunkPosition cPos = GetChunkPosition(tilemap, player.tileX, player.tileY);
	chunks[0] = GetChunk(tilemap, cPos.chunkX - 1, cPos.chunkY + 1);
	chunks[1] = GetChunk(tilemap, cPos.chunkX, cPos.chunkY + 1);
	chunks[2] = GetChunk(tilemap, cPos.chunkX + 1, cPos.chunkY + 1);
	chunks[3] = GetChunk(tilemap, cPos.chunkX - 1, cPos.chunkY);
	chunks[4] = GetChunk(tilemap, cPos.chunkX, cPos.chunkY);
	chunks[5] = GetChunk(tilemap, cPos.chunkX + 1, cPos.chunkY);
	chunks[6] = GetChunk(tilemap, cPos.chunkX - 1, cPos.chunkY - 1);
	chunks[7] = GetChunk(tilemap, cPos.chunkX, cPos.chunkY - 1);
	chunks[8] = GetChunk(tilemap, cPos.chunkX + 1, cPos.chunkY - 1);

	for (u32 chunkY = 0; chunkY < 3; chunkY++)
	{
		for (u32 chunkX = 0; chunkX < 3; chunkX++)
		{
			for (u32 row = 0; row < tilemap->chunkSizeInTiles; row++)
			{
				for (u32 col = 0; col < tilemap->chunkSizeInTiles; col++)
				{
					u32 tileValue = GetTileValueInChunk(tilemap,
														chunks[chunkY * 3 + chunkX],
														col, row);
					if (tileValue)
					{
						v3 color;
						f32 yOffset = 0.0f;
						if (GetTileValueInChunk(tilemap,
												chunks[chunkY * 3 + chunkX],
												col, row) == 3)
						{
							yOffset = 2.0f;
						}
						else
						{
							yOffset = 0.0f;
						}
						color = GetTileColor(tilemap,
											 chunks[chunkY * 3 + chunkX],
											 col, row);
						ChunkPosition playerCPos =
							GetChunkPosition(tilemap,
											 player.tileX, player.tileY);
						if (row == playerCPos.tileInChunkY &&
							col == playerCPos.tileInChunkX)// &&
							//chunkX == playerCPos.chunkX &&
							//chunkY == playerCPos.chunkY)
						{
							color = V3(0.0f);
						}
			

						f32	chunkOffsetX = chunkX * tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw - tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw;
						f32 chunkOffsetY = chunkY * tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw - tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw;
					
						f32 zOffset = row *	(f32)tilemap->tileSizeRaw -
							(tilemap->tileRadiusInUnits * tilemap->unitsToRaw);

						f32 xOffset = col * (f32)tilemap->tileSizeRaw -
							(tilemap->tileRadiusInUnits * tilemap->unitsToRaw);

						f32 zInvBias = tilemap->chunkSizeInTiles *
							(f32)tilemap->tileSizeRaw; 

						v3 position = V3(xOffset + chunkOffsetX,
										 yOffset,
										 (-zOffset + zInvBias) + chunkOffsetY);

						DrawDebugCube(renderGroup,
									  assetManager,
									  position, tilemap->tileSizeRaw * 0.5f, color);
					}
				}
			}
		}
	}
}
#else
void DrawWorld(const Tilemap* tilemap, RenderGroup* renderGroup,
			   AssetManager* assetManager, TilemapPosition player,
			   u32 negChunkCountX, u32 negChunkCountY,
			   u32 posChunkCountX, u32 posChunkCountY)
{
	ChunkPosition pChunkPos = GetChunkPosition(tilemap, player.tileX,
											   player.tileY); 

	u32 chunkCountX = negChunkCountX + posChunkCountX + 1;
	u32 chunkCountY = negChunkCountY + posChunkCountY + 1;

	v2 origin = V2(0.0f);
	origin.x = -1.0f * negChunkCountX * tilemap->chunkSizeInTiles
		* tilemap->tileSizeInUnits
		- pChunkPos.tileInChunkX * tilemap->tileSizeInUnits
		- player.offset.x;
	origin.y = -1.0f * negChunkCountY * tilemap->chunkSizeInTiles
		* tilemap->tileSizeInUnits
		+ (tilemap->chunkSizeInTiles - pChunkPos.tileInChunkY)
		* tilemap->tileSizeInUnits - ( player.offset.y);

	// NOTE: i32 is ok here. There is no way it will
	// render more than 2 billion chunks
	// TODO: Clamp at max coord
	u32 beginChunkX;
	u32 biasX = 0;
	u32 biasY = 0;
	if (negChunkCountX > pChunkPos.chunkX)
	{
		beginChunkX = 0;
		biasX = negChunkCountX - pChunkPos.chunkX;
	}
	else
	{
		beginChunkX = pChunkPos.chunkX - negChunkCountX;
	}
		
	u32 beginChunkY;
	if (negChunkCountY > pChunkPos.chunkY)
	{
		beginChunkY = 0;
		biasY = negChunkCountY - pChunkPos.chunkY;
	}
	else
	{
		beginChunkY = pChunkPos.chunkY - negChunkCountY;				
	}

	chunkCountX -= biasX;
	chunkCountY -= biasY;

	for (u32 chunkY = beginChunkY;
		 chunkY < beginChunkY + chunkCountY;
		 chunkY++)
	{
		for (u32 chunkX = beginChunkX;
			 chunkX < beginChunkX + chunkCountX;
			 chunkX++)
		{
			Chunk* chunk = GetChunk(tilemap,
									chunkX, chunkY);

			f32 chunkOffsetX = origin.x + (chunkX - beginChunkX + biasX) *
				tilemap->chunkSizeInTiles * tilemap->tileSizeInUnits;
			f32 chunkOffsetY = origin.y + (chunkY - beginChunkY + biasY) *
				tilemap->chunkSizeInTiles * tilemap->tileSizeInUnits;

			for (u32 row = 0; row < tilemap->chunkSizeInTiles; row++)
			{
				for (u32 col = 0; col < tilemap->chunkSizeInTiles; col++)
				{
					u32 tileValue = GetTileValueInChunk(tilemap,
														chunk,
														col, row);
					if (tileValue)
					{
						v3 color;
						f32 yOffset = 0.0f;
						if (GetTileValueInChunk(tilemap,
												chunk,
												col, row) == 3)
						{
							yOffset = 2.0f;
						}
						else
						{
							yOffset = 0.0f;
						}
						color = GetTileColor(tilemap,
											 chunk,
											 col, row);
						if (row == pChunkPos.tileInChunkY &&
							col == pChunkPos.tileInChunkX)// &&
							//chunkX == playerCPos.chunkX &&
							//chunkY == playerCPos.chunkY)
						{
							color = V3(0.0f);
						}
	  
						f32 zOffset = row *
							(f32)tilemap->tileSizeInUnits -
							tilemap->tileRadiusInUnits;

						f32 xOffset = col *
							(f32)tilemap->tileSizeInUnits -
							tilemap->tileRadiusInUnits;

						f32 zInvBias = tilemap->chunkSizeInTiles *
							(f32)tilemap->tileSizeInUnits; 

						v3 position = V3(0.0f);
						position.x = (xOffset + chunkOffsetX)
							* tilemap->unitsToRaw;
						position.y = yOffset;
						position.z = (-zOffset + zInvBias - chunkOffsetY)
							* tilemap->unitsToRaw;

						DrawDebugCube(renderGroup,
									  assetManager,
									  position, tilemap->tileSizeRaw * 0.5f, color);
					}
				}
			}
		}
	}

#if 0	
	for (u32 chunkY = minChunkY; chunkY <= maxChunkX; chunkY++) 
	{
		for (u32 chunkX = minChunkX; chunkX <= maxChunkX; chunkX++)
		{
			for (u32 row = 0; row < tilemap->chunkSizeInTiles; row++)
			{
				for (u32 col = 0; col < tilemap->chunkSizeInTiles; col++)
				{
					Chunk* chunk = GetChunk(tilemap, chunkX, chunkY);
					u32 tileValue = GetTileValueInChunk(tilemap,
														chunk,
														col, row);
					if (tileValue)
					{
						v3 color;
						f32 yOffset = 0.0f;
						if (GetTileValueInChunk(tilemap,
												chunk,
												col, row) == 3)
						{
							yOffset = 2.0f;
						}
						else
						{
							yOffset = 0.0f;
						}
						color = GetTileColor(tilemap,
											 chunk,
											 col, row);
						ChunkPosition playerCPos =
							GetChunkPosition(tilemap,
											 player.tileX, player.tileY);
						if (row == playerCPos.tileInChunkY &&
							col == playerCPos.tileInChunkX)// &&
							//chunkX == playerCPos.chunkX &&
							//chunkY == playerCPos.chunkY)
						{
							color = V3(0.0f);
						}
			

						f32	chunkOffsetX = chunkX * tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw - tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw;
						f32 chunkOffsetY = chunkY * tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw - tilemap->chunkSizeInTiles *
							tilemap->tileSizeRaw;
					
						f32 zOffset = row *	(f32)tilemap->tileSizeRaw -
							(tilemap->tileRadiusInUnits * tilemap->unitsToRaw);

						f32 xOffset = col * (f32)tilemap->tileSizeRaw -
							(tilemap->tileRadiusInUnits * tilemap->unitsToRaw);

						f32 zInvBias = tilemap->chunkSizeInTiles *
							(f32)tilemap->tileSizeRaw; 

						v3 position = V3(xOffset + chunkOffsetX,
										 yOffset,
										 (-zOffset + zInvBias) + chunkOffsetY);

						DrawDebugCube(renderGroup,
									  assetManager,
									  position, tilemap->tileSizeRaw * 0.5f, color);
					}
				}
			}
		}
	}
#endif
}
#endif

// NOTE: Frustum stuff
		FrustumVertices camFV = {};
		m3x3 camRot = M3x3(sandbox->renderGroup->camera.lookAt);
		GenFrustumVertices(&camRot, sandbox->camera.pos, 0.1f, 200.0f, 45.0f,
						   16.0f / 9.0f, &camFV);

		// NOTE: Vectors from frustum
		v3 lt = camFV.farLeftTop - camFV.nearLeftTop;
		v3 lb = camFV.farLeftBottom - camFV.nearLeftBottom;
		v3 rb = camFV.farRightBottom - camFV.nearRightBottom;
		v3 rt = camFV.farRightTop - camFV.nearRightTop;

		f32 frustumMinX = camFV.vertices[0].x;
		f32 frustumMinY = camFV.vertices[0].y;
		f32 frustumMaxX = camFV.vertices[0].x;
		f32 frustumMaxY = camFV.vertices[0].y;
		   
		for (u32 i = 1; i < 8; i++)
		{
			if (camFV.vertices[i].x < frustumMinX)
			{
				frustumMinX = camFV.vertices[i].x;
			}
			if (camFV.vertices[i].x > frustumMaxX)
			{
				frustumMaxX = camFV.vertices[i].x;
			}
			if (camFV.vertices[i].y < frustumMinY)
			{
				frustumMinY = camFV.vertices[i].y;
			}
			if (camFV.vertices[i].y > frustumMaxY)
			{
				frustumMaxY = camFV.vertices[i].y;
			}
		}
		DEBUG_OVERLAY_TRACE_VAR(frustumMinX);
		DEBUG_OVERLAY_TRACE_VAR(frustumMaxX);
		DEBUG_OVERLAY_TRACE_VAR(frustumMinY);
		DEBUG_OVERLAY_TRACE_VAR(frustumMaxY);
		frustumMinX *= tilemap->toUnits;
		frustumMaxX *= tilemap->toUnits;
		frustumMinY *= tilemap->toUnits;
		frustumMaxY *= tilemap->toUnits;
		
#if defined(RAYCAST_TEST)

		v3 relCamPos = sandbox->camera.pos - sandbox->camera.target;
		
		i32 ltTileX = 0;
		i32 ltTileY = 0;
		b32 hitTerrain = RaycastToGround(tilemap, relCamPos, lt,
										 &ltTileX, &ltTileY);

#define _SIGN(a) (SignBit(a) == 0 ? 1 : -1)
		
		if (!hitTerrain)
		{
			ltTileX = 201 * _SIGN(lt.x);
			ltTileY = 201 * _SIGN(lt.z);
		}

		i32 lbTileX = 0;
		i32 lbTileY = 0;
		hitTerrain = RaycastToGround(tilemap, relCamPos, lb,
									 &lbTileX, &lbTileY);

		if (!hitTerrain)
		{
			lbTileX = 201 * _SIGN(lb.x);
			lbTileY = 201 * _SIGN(lb.z);
		}


		DEBUG_OVERLAY_TRACE_VAR(relCamPos);
		DEBUG_OVERLAY_TRACE_VAR(rt);
		i32 rtTileX = 0;
		i32 rtTileY = 0;
		hitTerrain = RaycastToGround(tilemap, relCamPos, rt,
									 &rtTileX, &rtTileY);

		if (!hitTerrain)
		{
			rtTileX = 201 * _SIGN(rt.x);
			rtTileY = 201 * _SIGN(rt.z);
		}


		i32 rbTileX = 0;
		i32 rbTileY = 0;
		hitTerrain = RaycastToGround(tilemap, relCamPos, rb,
									 &rbTileX, &rbTileY);

		if (!hitTerrain)
		{
			rbTileX = 201 * _SIGN(rb.x);
			rbTileY = 201 * _SIGN(rb.z);
		}


		i32 minOffsetX = MINIMUM(MINIMUM(ltTileX, lbTileX), MINIMUM(rtTileX, rbTileX));
		i32 minOffsetY = MINIMUM(MINIMUM(ltTileY, lbTileY), MINIMUM(rtTileY, rbTileY));
		i32 maxOffsetX = MAXIMUM(MAXIMUM(ltTileX, lbTileX), MAXIMUM(rtTileX, rbTileX));
		i32 maxOffsetY = MAXIMUM(MAXIMUM(ltTileY, lbTileY), MAXIMUM(rtTileY, rbTileY));
		if (minOffsetX > 200)
		{
			minOffsetX = 200;
		}
		if (minOffsetX < -200)
		{
			minOffsetX = -200;
		}
		if (maxOffsetX > 200)
		{
			maxOffsetX = 200;
		}
		if (maxOffsetX < -200)
		{
			maxOffsetX = -200;
		}
		if (minOffsetY > 200)
		{
			minOffsetY = 200;
		}
		if (minOffsetY < -200)
		{
			minOffsetY = -200;
		}
		if (maxOffsetY > 200)
		{
			maxOffsetY = 200;
		}
		if (maxOffsetY < -200)
		{
			maxOffsetY = -200;
		}

		minOffsetX = Floor((f32)minOffsetX / tilemap->chunkSizeInTiles);
		minOffsetY = Floor((f32)minOffsetY / tilemap->chunkSizeInTiles);
		maxOffsetX = Ceil((f32)maxOffsetX / tilemap->chunkSizeInTiles);
		maxOffsetY = Ceil((f32)maxOffsetY / tilemap->chunkSizeInTiles);

		DEBUG_OVERLAY_TRACE_VAR(minOffsetX);
		DEBUG_OVERLAY_TRACE_VAR(maxOffsetX);
		DEBUG_OVERLAY_TRACE_VAR(minOffsetY);
		DEBUG_OVERLAY_TRACE_VAR(maxOffsetY);
#else

		f32 chunkSizeUnits = tilemap->chunkSizeInTiles * tilemap->tileSizeInUnits;

		i32 minChunkOffsetX = RoundF32I32(frustumMinX / chunkSizeUnits);
		i32 maxChunkOffsetX = RoundF32I32(frustumMaxX / chunkSizeUnits);
		i32 minChunkOffsetY = RoundF32I32(frustumMinY / chunkSizeUnits);
		i32 maxChunkOffsetY = RoundF32I32(frustumMaxX / chunkSizeUnits);
#endif


	b32 RaycastToGround(const Tilemap* tilemap, v3 from, v3 dir, 
						i32* hitTileOffsetX, i32* hitTileOffsetY)
	{
		// NOTE: Returns offset relative to from
		b32 result = false;
		if (dir.y < 0.0f)
		{
			f32 tIntersection =  -from.y / dir.y;
			f32 xIntersection = from.x + dir.x * tIntersection;
			f32 zIntersection = from.z + dir.z * tIntersection;
			v3 intersectionCoord = V3(xIntersection, 0.0f, zIntersection);
			intersectionCoord *= tilemap->toUnits;
			i32 tileOffsetX = TruncF32I32(intersectionCoord.x /
										  tilemap->tileSizeInUnits);
			i32 tileOffsetY = -TruncF32I32(intersectionCoord.z /
										   tilemap->tileSizeInUnits);
			result = true;
			*hitTileOffsetX = tileOffsetX;
			*hitTileOffsetY = tileOffsetY;
		}
		return result;
	}


#endif
	
#if 0
	void UpdateInput(AnnoCamera* camera)
	{
		i32 forwardMovement = 0;
		if (InputMouseButtonIsDown(KeyboardKey::W))
		{
			forwardMovement += 1;
		}
		if (InputMouseButtonIsDown(KeyboardKey::S))
		{
			forwardMovement -= 1;
		}
	}
#endif
#if 0
	void SubscribeKeyboardEvents(AnnoCamera* camera,
								 InputManager* inputManager,
								 InputState* inputState)
	{
		auto CameraMovementCallback = [](AB::Event e)
			{
				int32 action = -1;
				if (e.type == AB::EVENT_TYPE_KEY_PRESSED)
				{
					action = 1;
				}
				else if (e.type == AB::EVENT_TYPE_KEY_RELEASED)
				{
					action = 0;
				}
	
				if (action != -1)
				{
					switch (e.key_event.key)
					{
					case AB::KeyboardKey::W:
					{
						camera->frameMovementFlags.forward = action;
					} break;
					case AB::KeyboardKey::S:
					{
						camera->frameMovementFlags.back = action;
					} break;
					case AB::KeyboardKey::A: {
						camera->frameMovementFlags.left = action;
					} break;
					case AB::KeyboardKey::D: {
						camera->frameMovementFlags.right = action;
					} break;
					INVALID_DEFAULT_CASE();
					}
				}
			};

		EventQuery w = {};
		w.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		w.condition.key_event.key = KeyboardKey::W;
		w.callback = CameraMovementCallback;

		AB::EventQuery s = {};
		s.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		s.condition.key_event.key = KeyboardKey::S;
		s.callback = CameraMovementCallback;

		AB::EventQuery a = {};
		a.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		a.condition.key_event.key = KeyboardKey::A;
		a.callback = CameraMovementCallback;

		AB::EventQuery d = {};
		d.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		d.condition.key_event.key = KeyboardKey::D;
		d.callback = CameraMovementCallback;

		InputSubscribeEvent(inputManager, &w);
		InputSubscribeEvent(inputManager, &s);
		InputSubscribeEvent(inputManager, &a);
		InputSubscribeEvent(inputManager, &d);

		EventQuery tabEvent = {};
		tabEvent.type = EventType::EVENT_TYPE_KEY_PRESSED;
		tabEvent.condition.key_event.key = KeyboardKey::Tab;
		tabEvent.callback = [](Event e)
			{
				inputState.mouseCaptured = !inputState.mouseCaptured;
				if (inputState.mouseCaptured)
				{
					InputSetMouseMode(inputManager, MouseMode::Captured);
				}
				else
				{
					InputSetMouseMode(inputManager, MouseMode::Cursor);
				}
			};

		InputSubscribeEvent(inputManager, &tabEvent);

		EventQuery cursorEvent = {};
		cursorEvent.type = EVENT_TYPE_MOUSE_MOVED;
		cursorEvent.callback = [](Event e)
			{
				if (inputState.mouseCaptured)
				{
					camera->pitch += e.mouse_moved_event.y;
					//g_Camera.yaw += e.mouse_moved_event.x;
					inputState.lastMouseX = e.mouse_moved_event.x;
					inputState.lastMouseY = e.mouse_moved_event.y;
					if (camera->pitch > 89.0f)
					{
						camera->pitch = 89.0f;
					}
					if (camera->pitch < -89.0f)
					{
						camera->pitch = -89.0f;
					}
				}
			};

		InputSubscribeEvent(inputManager, &cursorEvent);
	}
#endif
#if 0 
    void DrawTile(Tilemap* tilemap, RenderGroup* renderGroup,
				  AssetManager* assetManager, TilemapPosition origin,
				  TilemapPosition tile)
	{
		ChunkPosition chunkOrigin = GetChunkPosition(tilemap, origin.tileX,
													 origin.tileY);
		ChunkPosition chunkTile = GetChunkPosition(tilemap, tile.tileX,
												   tile.tileY);

		u32 tileValue = GetTileValue(tilemap, tile.tileX, tile.tileY);
		if (tileValue)
		{
			i32 chunkDiffX = chunkTile.chunkX - chunkOrigin.chunkX;
			i32 chunkDiffY = chunkTile.chunkY - chunkOrigin.chunkY;
			i32 tileDiffX = chunkTile.tileInChunkX - chunkOrigin.tileInChunkX;
			i32 tileDiffY = chunkTile.tileInChunkY - chunkOrigin.tileInChunkY;
			f32 x = ((chunkDiffX * (i32)tilemap->chunkSizeInTiles + tileDiffX) *
					 tilemap->tileSizeInUnits - origin.offset.x - tilemap->tileRadiusInUnits) *
				tilemap->unitsToRaw;
			f32 y = -((chunkDiffY * (i32)tilemap->chunkSizeInTiles + tileDiffY) *
					  tilemap->tileSizeInUnits - origin.offset.y - tilemap->tileRadiusInUnits) * tilemap->unitsToRaw;

			// TODO: Thus is temporary
			v3 color;
			f32 yOffset = 0.0f;
			if (tileValue == 3)
			{
				yOffset = 2.0f;
			}
			else
			{
				yOffset = 0.0f;
			}
			Chunk* chunk = GetChunk(tilemap, chunkTile.chunkX, chunkTile.chunkY);
			color = GetTileColor(tilemap,  chunk, chunkTile.tileInChunkX,
								 chunkTile.tileInChunkY);

			DrawDebugCube(renderGroup,
						  assetManager,
						  V3(x, yOffset, y), tilemap->tileSizeRaw * 0.5f, color);
		}
	}


void DrawWorld(Tilemap* tilemap, RenderGroup* renderGroup,
			   AssetManager* assetManager, TilemapPosition origin,
			   u32 w, u32 h)
{
	u32 width = tilemap->chunkSizeInTiles * w;
	u32 height = tilemap->chunkSizeInTiles * h;

	// TODO: SafeSub at the end of the world
	u32 beginX = SafeSub(origin.tileX, width / 2);
	u32 beginY = SafeSub(origin.tileY,  height / 2);

	u32 endX = SafeAdd(origin.tileX, width / 2);
	u32 endY = SafeAdd(origin.tileY, height / 2);


	for (u32 y = beginY; y < endY; y++)
	{
		for (u32 x = beginX; x < endX; x++)
		{
			DrawTile(tilemap, renderGroup,
					 assetManager, origin,
					 TilemapPosition{x, y});

		}
	}

}
	void DrawWorldInstanced(Tilemap* tilemap, RenderGroup* renderGroup,
							AssetManager* assetManager, TilemapPosition origin,
							u32 w, u32 h, i32 minX, i32 maxX, i32 minY, i32 maxY)
	{
		u32 width = w;
		u32 height = h;

		// TODO: SafeSub at the end of the world
		u32 beginX = SafeSub(origin.tileX, width / 2);// - offsetX;
		u32 beginY = SafeSub(origin.tileY,  height / 2);// - offsetY;

		u32 endX = SafeAdd(origin.tileX, width / 2);// - offsetX;
		u32 endY = SafeAdd(origin.tileY, height / 2);// - offsetY;

		RenderCommandBeginDebugCubeInctancing begCommand = {};
		begCommand.blendMode = BLEND_MODE_OPAQUE;

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_BEGIN_DEBUG_CUBE_INSTANCING,
							   (void*)(&begCommand));

		for (u32 y = beginY; y < endY; y++)
		{
			for (u32 x = beginX; x < endX; x++)
			{
				DrawTileInstanced(tilemap, renderGroup,
								  assetManager, origin,
								  TilemapPosition{x, y}, nullptr);

			}
		}

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING,
							   nullptr);
	}
	
#endif
