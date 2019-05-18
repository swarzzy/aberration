#include "Sandbox.h"
// TODO: This is temporary for rand()
#include <stdlib.h>

namespace AB
{
	void DrawDebugCube(RenderGroup* renderGroup,
					   AssetManager* assetManager,
					   v3 position, f32 scale, v3 color)
	{
		RenderCommandDrawDebugCube cubeCommand = {};
		m4x4 world = Identity4();
		world = Translate(world, position);
		world = Scale(world, V3(scale));
		cubeCommand.transform.worldMatrix = world;
		cubeCommand.color = color;

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_DEBUG_CUBE,
							   (void*)(&cubeCommand));

	}

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

	inline u32 SafeSub(u32 a, u32 b)
	{
		u32 result;
		if (b > a)
		{
			result = 0;
		}
		else
		{
			result = a - b;
		}
		return result;
	}

	inline u32 SafeAdd(u32 a, u32 b)
	{
		u32 result;
		if (a + b < a)
		{
			result = 0xffffffff;
		}
		else
		{
			result = a + b;
		}
		return result;
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

	void UpdateCamera(AnnoCamera* cam,
					  RenderGroup* renderGroup, Tilemap* tilemap);

	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  Sandbox* sandbox,
			  AssetManager* assetManager)
	{
		sandbox->renderGroup = AllocateRenderGroup(arena, MEGABYTES(300),
												   128000, 32);
		sandbox->renderGroup->projectionMatrix = PerspectiveRH(45.0f,
															   16.0f / 9.0f,
															   0.1f,
															   300.0f);


		sandbox->dirLight.ambient = V3(0.05f);
		sandbox->dirLight.diffuse = V3(1.1f);
		sandbox->dirLightOffset = V3(-10, 38, 17);
		Frustum frustum = FrustumFromProjRH(&PerspectiveRH(45.0f,
														   16.0f / 9.0f,
														   0.1f,
														   1.0f));
		m4x4 persp = sandbox->renderGroup->projectionMatrix;

		b32 result = false;
		v3 out = {};
		result = IntersectPlanes3(frustum.leftPlane,
								  frustum.nearPlane, frustum.bottomPlane, &out);

		result = GenFrustumVertices(&frustum, &sandbox->camFrustum);
		AB_ASSERT(result);
		
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
	
		//SubscribeKeyboardEvents(&sandbox->camera, inputManager,
		//						&sandbox->inputState);

		sandbox->gamma = 2.2f;
		sandbox->exposure = 1.0f;

		sandbox->playerP.tileX = 16 * 2 + 3;
		sandbox->playerP.tileY = 16 * 2 + 3;
		sandbox->playerP.offset.x = 1.0f;
		sandbox->playerP.offset.y = 1.0f;

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
	
	static b32 TestWall(f32 wallX, f32 relPlayerX, f32 relPlayerY,
						f32 playerDeltaX, f32 playerDeltaY,
						f32 minCornerY, f32 maxCornerY, f32* tMin)
	{
		b32 collided = false;
		if (AbsF32(playerDeltaX) > FLOAT_EPS)
		{
			f32 tEps = 0.01f;
			f32 tResult = (wallX - relPlayerX) / playerDeltaX;
			f32 y = relPlayerY + playerDeltaY * tResult;
			if ((tResult >= 0.0f) && (*tMin > tResult))
			{
				if ((y >= minCornerY) && (y <= maxCornerY))
				{
					*tMin = MAXIMUM(0.0f, tResult - tEps);
					collided = true;
				}
			}
		}
		return collided;
	}
	
	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer)
	{
		DEBUG_OVERLAY_SLIDER(g_Platform->gameSpeed, 0.0f, 10.0f);
		Tilemap* tilemap = &sandbox->world->tilemap;
		v3 _frontDir = V3(sandbox->camera.front.x, 0.0f, sandbox->camera.front.z);
		// NOTE: Do we need normalaze that
		_frontDir = Normalize(_frontDir);
		// NOTE: Maybe store it in camera explicitly
		v3 _rightDir = Cross(_frontDir, V3(0.0f, 1.0f, 0.0f));
		_rightDir = Normalize(_rightDir);

		// NOTE: Neagating Z because tilemap coords have Y axis pointing down-to-up
		// while right handed actual world coords have it
		// pointing opposite directhion.
		v2 frontDir = V2(_frontDir.x, -_frontDir.z);
		v2 rightDir = V2(_rightDir.x, -_rightDir.z);

		DEBUG_OVERLAY_TRACE_VAR(frontDir);
		DEBUG_OVERLAY_TRACE_VAR(rightDir);
		v2 playerAcceleration = {};
		if (!sandbox->camera.debugMode)
		{
			if (GlobalInput.keys[KEY_W].pressedNow)
			{
				playerAcceleration += frontDir;
			}
			if (GlobalInput.keys[KEY_S].pressedNow)
			{
				playerAcceleration -= frontDir;
			}
			if (GlobalInput.keys[KEY_A].pressedNow)
			{
				playerAcceleration -= rightDir;
			}
			if (GlobalInput.keys[KEY_D].pressedNow)
			{
				playerAcceleration += rightDir;
			}
		}

		playerAcceleration = Normalize(playerAcceleration);
		f32 playerSpeed = 20.0f;
		playerAcceleration *= playerSpeed;

		f32 friction = 3.0f;
		playerAcceleration = playerAcceleration - sandbox->playerSpeed * friction;

		v2 playerDelta;
		playerDelta = 0.5f * playerAcceleration *
			Square(GlobalGameDeltaTime) + 
			sandbox->playerSpeed *
			GlobalGameDeltaTime;

		TilemapPosition newPlayerPos = sandbox->playerP;

		newPlayerPos = OffsetTilemapPos(tilemap, newPlayerPos, playerDelta);

		v2 colliderSize = V2(tilemap->tileSizeInUnits,
							 tilemap->tileSizeInUnits);

		u32 colliderTileWidth = Ceil(colliderSize.x / tilemap->tileSizeInUnits);
		u32 colliderTileHeight = Ceil(colliderSize.y / tilemap->tileSizeInUnits);

		u32 minTileY = MINIMUM(sandbox->playerP.tileY, newPlayerPos.tileY);
		u32 maxTileY = MAXIMUM(sandbox->playerP.tileY, newPlayerPos.tileY);
		u32 minTileX = MINIMUM(sandbox->playerP.tileX, newPlayerPos.tileX);
		u32 maxTileX = MAXIMUM(sandbox->playerP.tileX, newPlayerPos.tileX);

		if (minTileY >= colliderTileHeight)
		{
			minTileY -= colliderTileHeight;
		}
		if (minTileX >= colliderTileWidth)
		{
			minTileX -= colliderTileWidth;			
		}
		// TODO: Prevent overflow at the end of the world
		maxTileY += colliderTileHeight;
		maxTileX += colliderTileWidth;

		b32 hit = false;
		f32 tRemaining = 1.0f;
		for (u32 pass = 0; (pass < 4) && (tRemaining > 0.0f); pass++)
		{
			// TODO: Temporary offseting for collision detection
			auto testingPos = sandbox->playerP;//OffsetTilemapPos(tilemap, sandbox->playerP,
			//			   V2(tilemap->tileSizeInUnits * 0.5f));

			v2 wallNormal = V2(0.0f);
			f32 tMin = 1.0f;
			for (u32 tileY = minTileY; tileY <= maxTileY; tileY++)
			{
				for (u32 tileX = minTileX; tileX <= maxTileX; tileX++)
				{
					u32 tileValue = GetTileValue(tilemap, tileX, tileY);
					TilemapPosition testTilePos = CenteredTilePoint(tileX, tileY);
					if (tileValue != 1)
					{
						v2 minCorner = -0.5f * V2(tilemap->tileSizeInUnits);
						v2 maxCorner = 0.5f * V2(tilemap->tileSizeInUnits);
						minCorner -= colliderSize;
						//maxCorner += colliderSize;
						v2 relOldPlayerPos = TilemapPosDiff(tilemap, &testingPos,
															&testTilePos);

						if (TestWall(minCorner.x, relOldPlayerPos.x,
									 relOldPlayerPos.y, playerDelta.x,
									 playerDelta.y,
									 minCorner.y, maxCorner.y, &tMin))
						{
							wallNormal = V2(1.0f, 0.0f);
							hit = true;
						}
						
						if(TestWall(maxCorner.x, relOldPlayerPos.x,
									relOldPlayerPos.y, playerDelta.x,
									playerDelta.y,
									minCorner.y, maxCorner.y, &tMin))
						{
							wallNormal = V2(-1.0f, 0.0f);
							hit = true;
						}
						if(TestWall(minCorner.y, relOldPlayerPos.y,
									relOldPlayerPos.x, playerDelta.y,
									playerDelta.x,
									minCorner.x, maxCorner.x, &tMin))
						{
							wallNormal = V2(0.0f, 1.0f);
							hit = true;
						}
						if(TestWall(maxCorner.y, relOldPlayerPos.y,
									relOldPlayerPos.x, playerDelta.y,
									playerDelta.x,
									minCorner.x, maxCorner.x, &tMin))
						{
							wallNormal = V2(0.0f, -1.0f);
							hit = true;
						}
					}
				}
			}
			v2 playerFrameOffset = playerDelta * tMin;
			sandbox->playerP = OffsetTilemapPos(tilemap, sandbox->playerP,
												playerFrameOffset);
			if (hit)
			{
				tRemaining -= tMin * tRemaining;
				sandbox->playerSpeed = sandbox->playerSpeed -
					Dot(sandbox->playerSpeed, wallNormal) * wallNormal;
				playerDelta *= 1.0f - tMin;
				playerDelta = playerDelta -
					Dot(playerDelta, wallNormal) * wallNormal;				
			}
			else
			{
				break;
			}
		}

		//sandbox->playerSpeed = Reflect(sandbox->playerSpeed,  wallNormal);

		sandbox->playerSpeed = sandbox->playerSpeed +
			playerAcceleration * GlobalGameDeltaTime;

		ChunkPosition chunkPos = GetChunkPosition(tilemap, sandbox->playerP.tileX,
												  sandbox->playerP.tileY);

		UpdateCamera(&sandbox->camera,
					 sandbox->renderGroup, tilemap);

		FrustumVertices camFV = {};
		m3x3 camRot = M3x3(sandbox->camera.lookAt);
		v3 camPos = (sandbox->camera.pos - sandbox->camera.target) * tilemap->unitsToRaw;
		DEBUG_OVERLAY_TRACE_VAR(camPos);
		GenFrustumVertices(&camRot, camPos,
						   0.1f, 300.0f, 45.0f,
						   16.0f / 9.0f, &camFV);

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
			if (camFV.vertices[i].z < frustumMinY)
			{
				frustumMinY = camFV.vertices[i].z;
			}
			if (camFV.vertices[i].z > frustumMaxY)
			{
				frustumMaxY = camFV.vertices[i].z;
			}
		}
		f32 chunkSize = tilemap->chunkSizeInTiles * tilemap->tileSizeInUnits;
#if 1
		frustumMinX *= tilemap->toUnits;
		frustumMaxX *= tilemap->toUnits;
		frustumMinY *= tilemap->toUnits;
		frustumMaxY *= tilemap->toUnits;
#endif
		DEBUG_OVERLAY_TRACE_VAR(frustumMinX);
		DEBUG_OVERLAY_TRACE_VAR(frustumMaxX);
		DEBUG_OVERLAY_TRACE_VAR(frustumMinY);
		DEBUG_OVERLAY_TRACE_VAR(frustumMaxY);


		i32 chunkMinX = RoundF32I32(frustumMinX / chunkSize + 0.5f);
		i32 chunkMaxX = RoundF32I32(frustumMaxX / chunkSize + 0.5f);
		i32 chunkMinY = RoundF32I32(frustumMinY / chunkSize + 0.5f);
		i32 chunkMaxY = RoundF32I32(frustumMaxY / chunkSize + 0.5f);

		DEBUG_OVERLAY_TRACE_VAR(chunkMinX);
		DEBUG_OVERLAY_TRACE_VAR(chunkMaxX);
		DEBUG_OVERLAY_TRACE_VAR(chunkMinY);
		DEBUG_OVERLAY_TRACE_VAR(chunkMaxY);

		u32 w = AbsI32(chunkMaxX - chunkMinX);
		u32 h = AbsI32(chunkMaxY - chunkMinY);
		DEBUG_OVERLAY_TRACE_VAR(w);
		DEBUG_OVERLAY_TRACE_VAR(h);

		// Traverse all prefetched chunks and check for frustum - box intersection
		
		DrawWorld(tilemap, sandbox->renderGroup, assetManager,
				  sandbox->playerP, w - 2, h - 2);

		f32 pX = (chunkPos.tileInChunkX * tilemap->tileSizeInUnits +
				  sandbox->playerP.offset.x) * tilemap->unitsToRaw;
		f32 pY = (chunkPos.tileInChunkY * tilemap->tileSizeInUnits +
				  sandbox->playerP.offset.y) * tilemap->unitsToRaw;

		pY = (tilemap->chunkSizeInTiles * tilemap->tileSizeRaw) - pY;
			
		DrawDebugCube(sandbox->renderGroup,
					  assetManager,
					  V3(0.0f, 3.0f, 0.0f),
					  //V3(pX, 3.0f, pY),
					  tilemap->tileSizeRaw * 0.5f, V3(1.0f, 0.0f, 0.0f));
#if 1
		for (u32 i = 0; i < 8; i++)
		{
			DrawDebugCube(sandbox->renderGroup,
						  assetManager,
						  camFV.vertices[i],
						  8.0f, V3(0.0f, 0.0f, 1.0f)); 
		}
#endif

		//sandbox->camera.target = V3(pX, 0, pY);
		sandbox->dirLight.target = V3(pX, 0, pY);

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
#if 0

		DrawDebugCube(sandbox->renderGroup,
					  assetManager,
					  V3(0.0f), 1.0f, V3(1.0f, 0.0f, 0.0f));
		RenderGroupPushCommand(sandbox->renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_MESH,
							   (void*)(&planeCommand));
#endif

		f32 ka = sandbox->dirLight.ambient.r;
		f32 kd = sandbox->dirLight.diffuse.r;
	
		DEBUG_OVERLAY_PUSH_SLIDER("Ka", &ka, 0.0f, 1.0f);
		DEBUG_OVERLAY_PUSH_VAR("Ka", ka);

		DEBUG_OVERLAY_PUSH_SLIDER("Kd", &kd, 0.0f, 10.0f);
		DEBUG_OVERLAY_PUSH_VAR("Kd", kd);

		//DEBUG_OVERLAY_TRACE_VAR(sandbox->dirLight.from);
		//DEBUG_OVERLAY_TRACE_VAR(sandbox->dirLight.target);
		//DEBUG_OVERLAY_TRACE_VAR(sandbox->dirLightOffset);
		//DEBUG_OVERLAY_PUSH_SLIDER("offset", &sandbox->dirLightOffset, -50, 50);

		sandbox->dirLight.from = AddV3V3(sandbox->dirLight.target, sandbox->dirLightOffset);
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
	
	void UpdateCamera(AnnoCamera* cam,
					  RenderGroup* renderGroup, Tilemap* tilemap)
	{
		if (GlobalInput.keys[KEY_F1].pressedNow &&
			!GlobalInput.keys[KEY_F1].wasPressed)
		{
			cam->debugMode = !cam->debugMode;	
		}

		if (!cam->debugMode)
		{
#if 0
			if (cam->frameMovementFlags.forward)
			{
				cam->pos = AddV3V3(MulV3F32(cam->front, 0.2), cam->pos);
			}
			if (cam->frameMovementFlags.back)
			{
				cam->pos = SubV3V3(cam->pos, MulV3F32(cam->front, 0.2));
			}
			if (cam->frameMovementFlags.left)
			{
				v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
				cam->pos = SubV3V3(cam->pos, MulV3F32(right, 0.2));
			}
			if (cam->frameMovementFlags.right)
			{
				v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
				cam->pos = AddV3V3(MulV3F32(right, 0.2), cam->pos);
			}
#endif

			if (GlobalInput.mouseButtons[MBUTTON_RIGHT].pressedNow)
			{
 				v2 mousePos;
				mousePos.x = GlobalInput.mouseFrameOffsetX;
				mousePos.y = GlobalInput.mouseFrameOffsetY;
				cam->lastMousePos.x -= mousePos.x;// - cam->mouseUntrackedOffset;
				cam->lastMousePos.y -= mousePos.y;
				//DEBUG_OVERLAY_PUSH_VAR("mouse y:", mousePos.y);
			}

			if (cam->lastMousePos.y < 95.0f)
			{
				cam->lastMousePos.y = 95.0f;
				//cam->latitude = 95.000001f;
			}
			else if (cam->lastMousePos.y > 170.0f)
			{
				cam->lastMousePos.y = 170.0f;
				//cam->latitude = 170.00001f;
			}

		
			f32 scrollSpeed = 5.0f;
		
			i32 frameScrollOffset = GlobalInput.scrollFrameOffset;
			cam->targetDistance -= frameScrollOffset * scrollSpeed;

			if (cam->targetDistance < 5.0f)
			{
				cam->targetDistance = 5.0f;
			} else if (cam->targetDistance > 50.0f)
			{
				cam->targetDistance = 50.0f;
			}

			cam->latitude = Lerp(cam->latitude, cam->lastMousePos.y,
								 cam->latSmoothness);

			cam->longitude = Lerp(cam->longitude, cam->lastMousePos.x,
								  cam->longSmoothness);

			cam->distance = Lerp(cam->distance, cam->targetDistance,
								 cam->distSmoothness);


			//	DEBUG_OVERLAY_PUSH_SLIDER("r", &cam->distance, 1.0f, 10.0f);
			//DEBUG_OVERLAY_PUSH_VAR("r", cam->distance);

			///DEBUG_OVERLAY_PUSH_SLIDER("target", &cam->target, 0.0f, 10.0f);
			//DEBUG_OVERLAY_PUSH_VAR("target", cam->target);
			//DEBUG_OVERLAY_PUSH_SLIDER("height", &cam->latitude, 0.0f, 180.0f);
			//	DEBUG_OVERLAY_PUSH_VAR("height", cam->latitude);

			f32 latitude = ToRadians(cam->latitude);
			f32 longitude = ToRadians(cam->longitude);
			f32 polarAngle = PI_32 - latitude;

			f32 z = cam->target.z + cam->distance * Sin(polarAngle) * Cos(longitude);
			f32 x = cam->target.x + cam->distance * Sin(polarAngle) * Sin(longitude);
			f32 y = cam->target.y + cam->distance * Cos(polarAngle);

			cam->pos = V3(x, y, z);
		
			cam->front = Normalize(SubV3V3(cam->target, cam->pos));

			v3 pos = cam->pos * tilemap->unitsToRaw;
			v3 front = cam->front * tilemap->unitsToRaw;

			cam->lookAt = LookAtRH(pos,
								   AddV3V3(pos, front),
								   V3(0.0f, 1.0f, 0.0f));
		
			RenderGroupSetCamera(renderGroup,
								 // Normalization?
								 front, 
								 pos,
								 &cam->lookAt);
		}
		else
		{
			DEBUG_OVERLAY_PUSH_SLIDER("Debug camera speed:", &cam->debugSpeed,
									  0.0f, 10.0f);
			if (GlobalInput.keys[KEY_W].pressedNow)
			{
				cam->debugPos += cam->debugFront * GlobalAbsDeltaTime
					* cam->debugSpeed;
			}
			if (GlobalInput.keys[KEY_S].pressedNow)
			{
				cam->debugPos -= cam->debugFront * GlobalAbsDeltaTime
					* cam->debugSpeed;
			}
			if (GlobalInput.keys[KEY_A].pressedNow)
			{
				v3 right = Normalize(Cross(cam->debugFront, { 0, 1, 0 }));
				cam->debugPos -= right * GlobalAbsDeltaTime
					* cam->debugSpeed;
			}
			if (GlobalInput.keys[KEY_D].pressedNow)
			{
				v3 right = Normalize(Cross(cam->debugFront, { 0, 1, 0 }));
				cam->debugPos += right * GlobalAbsDeltaTime
					* cam->debugSpeed;
			}

			cam->debugPitch += GlobalInput.mouseFrameOffsetY;
			cam->debugYaw += GlobalInput.mouseFrameOffsetX;
			
			if (cam->debugPitch > 89.0f)
				cam->debugPitch = 89.0f;
			if (cam->debugPitch < -89.0f)
				cam->debugPitch = -89.0f;

			cam->debugFront.x = Cos(ToRadians(cam->debugPitch))
				* Cos(ToRadians(cam->debugYaw));
			cam->debugFront.y = Sin(ToRadians(cam->debugPitch));
			cam->debugFront.z = Cos(ToRadians(cam->debugPitch))
				* Sin(ToRadians(cam->debugYaw));
			cam->debugFront = Normalize(cam->debugFront);

			v3 pos = cam->debugPos * tilemap->unitsToRaw;
			v3 front = cam->debugFront * tilemap->unitsToRaw;
			
			cam->debugLookAt = LookAtRH(pos,
										AddV3V3(pos, front),
										V3(0.0f, 1.0f, 0.0f));
		
			RenderGroupSetCamera(renderGroup,
								 front,
								 pos,
								 &cam->debugLookAt);
		}

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
