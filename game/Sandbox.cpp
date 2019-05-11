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
#if 0
	void DrawChunk(Tilemap* tilemap, Chunk* chunk, RenderGroup* renderGroup,
				   AssetManager* assetManager, u32 playerTileX, u32 playerTileY)
	{

		for (u32 row = 0; row < tilemap->chunkSizeInTiles; row++)
		{
			for (u32 col = 0; col < tilemap->chunkSizeInTiles; col++)
			{
				v3 color;
				f32 yOffset;
				if (GetTileValue(tilemap, chunk, col, row) == 0)
				{
					color = V3(1.0f, 0.1f, 0.0f);
					yOffset = 0.0f;
				}
				else
				{
					color = V3(0.0f, 0.1f, 0.8f);
					yOffset = 1.0f;
				}
				if (row == playerTileY && col == playerTileX)
				{
					color = V3(0.0f);
				}
				color = GetTileColor(tilemap, chunk, col, row);

				f32 zOffset = tilemap->tilemapChunkCountY *
					(f32)tilemap->tileSizeRaw  - row * (f32)tilemap->tileSizeRaw +
					(tilemap->tileRadiusInUnits * tilemap->unitsToRaw);

				v3 position = V3(col * (f32)tilemap->tileSizeRaw - (tilemap->tileRadiusInUnits * tilemap->unitsToRaw),
								 yOffset,
								 zOffset);

				DrawDebugCube(renderGroup,
							  assetManager,
							  position, tilemap->tileSizeRaw * 0.5f, color);
			}
		}
	}
#else
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

#endif
	void SubscribeKeyboardEvents(AnnoCamera* camera,
								 InputManager* inputManager,
								 InputState* inputState);

	void UpdateCamera(AnnoCamera* cam, InputManager* inputManager);

	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  Sandbox* sandbox,
			  AssetManager* assetManager,
			  InputManager* inputManager)
	{
		sandbox->renderGroup = AllocateRenderGroup(arena, MEGABYTES(8),
												   4096, 32);
		sandbox->renderGroup->projectionMatrix = PerspectiveRH(45.0f,
															   16.0f / 9.0f,
															   0.1f,
															   200.0f);

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
	
		//SubscribeKeyboardEvents(&sandbox->camera, inputManager,
		//						&sandbox->inputState);

		sandbox->gamma = 2.2f;
		sandbox->exposure = 1.0f;

		sandbox->playerP.tileX = 2;
		sandbox->playerP.tileY = 2;
		sandbox->playerP.offset.x = 1.0f;
		sandbox->playerP.offset.y = 1.0f;

		sandbox->world = (World*)PushSize(arena, sizeof(World), alignof(World));
		AB_ASSERT(sandbox->world);

		Tilemap* tilemap = &(sandbox->world->tilemap);

		tilemap->tilemapChunkCountX = 8;
		tilemap->tilemapChunkCountY = 8;
		tilemap->chunkSizeInTiles = 16;
		tilemap->tileSizeRaw = 3.0f;
		tilemap->tileSizeInUnits = 1.0f;
		tilemap->tileRadiusInUnits = 0.5f;
		tilemap->toUnits = tilemap->tileSizeInUnits / tilemap->tileSizeRaw;
		tilemap->unitsToRaw = tilemap->tileSizeRaw / tilemap->tileSizeInUnits;
		tilemap->chunkMask = 0x0000000f;
		tilemap->chunkShift = 4;

		tilemap->chunks = (Chunk*)PushSize(arena,
										   tilemap->tilemapChunkCountX *
										   tilemap->tilemapChunkCountY *
										   sizeof(Chunk),
										   0);
		AB_ASSERT(tilemap->chunks);

		f32 r = 0.2f;
		f32 g = 0.2f;
		f32 b = 0.2f;
		
		for (u32 y = 0; y < tilemap->tilemapChunkCountX; y++)
		{
			for (u32 x = 0; x < tilemap->tilemapChunkCountY; x++)
			{
				r = rand() % 11 / 10.0f;
				g = rand() % 11 / 10.0f;
				b = rand() % 11 / 10.0f;
				Chunk* chunk = GetChunk(tilemap, x, y);
				for (u32 tileY = 0; tileY < tilemap->chunkSizeInTiles; tileY++)
				{
					for (u32 tileX = 0; tileX < tilemap->chunkSizeInTiles; tileX++)
					{
						if (x == y || x - 1 == y)
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
	

	}
	
	static f32 TestWall(f32 wallX, f32 relPlayerX, f32 relPlayerY,
						f32 playerDeltaX, f32 playerDeltaY, f32 tMin,
						f32 minCornerY, f32 maxCornerY)
	{
		if (Abs(playerDeltaX) > FLOAT_EPS)
		{
			f32 tEps = 0.001f;
			f32 tResult = (wallX - relPlayerX) / playerDeltaX;
			f32 y = relPlayerY + playerDeltaY * tResult;
			if ((tResult >= 0.0f) && (tMin > tResult))
			{
				if ((y >= minCornerY) && (y <= maxCornerY))
				{
					tMin = MAXIMUM(0.0f, tResult - tEps);
				}
			}
		}
		return tMin;
	}
	
	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer,
				InputManager* inputManager)
	{
		Tilemap* tilemap = &sandbox->world->tilemap;
		v3 _frontDir = V3(sandbox->camera.front.x, 0.0f, sandbox->camera.front.z);
		// NOTE: Do we need normalaze that
		_frontDir = Normalize(_frontDir);
		// NOTE: Maybe store it in camera explicitly
		v3 _rightDir = Cross(_frontDir, V3(0.0f, 1.0f, 0.0f));

		// NOTE: Neagating Z because tilemap coords have Y axis pointing down-to-up
		// while right handed actual world coords have it
		// pointing opposite directhion.
		v2 frontDir = V2(_frontDir.x, -_frontDir.z);
		v2 rightDir = V2(_rightDir.x, -_rightDir.z);

		DEBUG_OVERLAY_TRACE_VAR(frontDir);
		DEBUG_OVERLAY_TRACE_VAR(rightDir);
		v2 playerAcceleration = {};
		if (InputKeyIsDown(inputManager, KeyboardKey::W))
		{
			playerAcceleration += frontDir;
		}
		if (InputKeyIsDown(inputManager, KeyboardKey::S))
		{
			playerAcceleration -= frontDir;
		}
		if (InputKeyIsDown(inputManager, KeyboardKey::A))
		{
			playerAcceleration -= rightDir;
		}
		if (InputKeyIsDown(inputManager, KeyboardKey::D))
		{
			playerAcceleration += rightDir;
		}

		playerAcceleration = Normalize(playerAcceleration);
		f32 playerSpeed = 20.0f;
		playerAcceleration *= playerSpeed;

		f32 friction = 3.0f;
		playerAcceleration = playerAcceleration - sandbox->playerSpeed * friction;

		v2 playerDelta;
		playerDelta = 0.5f * playerAcceleration *
			Square(GlobalDeltaTime) + 
			sandbox->playerSpeed *
			GlobalDeltaTime;

		TilemapPosition oldPlayerPos = sandbox->playerP;
		TilemapPosition newPlayerPos = oldPlayerPos;
		newPlayerPos.offset += playerDelta;
		newPlayerPos = RecanonicalizePosition(tilemap, newPlayerPos);

		u32 minTileY = MINIMUM(oldPlayerPos.tileY, newPlayerPos.tileY);
		u32 onePastMaxTileY = MAXIMUM(oldPlayerPos.tileY, newPlayerPos.tileY) + 1;
		u32 minTileX = MINIMUM(oldPlayerPos.tileX, newPlayerPos.tileX);
		u32 onePastMaxTileX = MAXIMUM(oldPlayerPos.tileX, newPlayerPos.tileX) + 1;

		DEBUG_OVERLAY_TRACE_VAR(oldPlayerPos.tileX);
		DEBUG_OVERLAY_TRACE_VAR(newPlayerPos.tileX);
		
		f32 tMin = 1.0f;
		for (u32 tileY = minTileY; tileY != onePastMaxTileY; tileY++)
		{
			for (u32 tileX = minTileX; tileX != onePastMaxTileX; tileX++)
			{
				u32 tileValue = GetTileValue(tilemap, tileX, tileY);
				TilemapPosition testTilePos = CenteredTilePoint(tileX, tileY);
				if (tileValue != 1)
				{
					v2 minCorner = -0.5f * V2(tilemap->tileSizeInUnits);
					v2 maxCorner = 0.5f * V2(tilemap->tileSizeInUnits);
					v2 relOldPlayerPos = TilemapPosDiff(tilemap, &oldPlayerPos,
														 &testTilePos);

						tMin = TestWall(minCorner.x, relOldPlayerPos.x,
										relOldPlayerPos.y, playerDelta.x,
										playerDelta.y, tMin,
										minCorner.y, maxCorner.y);
						tMin = TestWall(maxCorner.x, relOldPlayerPos.x,
										relOldPlayerPos.y, playerDelta.x,
										playerDelta.y, tMin,
										minCorner.y, maxCorner.y);
						tMin = TestWall(minCorner.y, relOldPlayerPos.y,
										relOldPlayerPos.x, playerDelta.y,
										playerDelta.x, tMin,
										minCorner.x, maxCorner.x);
						tMin = TestWall(maxCorner.y, relOldPlayerPos.y,
										relOldPlayerPos.x, playerDelta.y,
										playerDelta.x, tMin,
										minCorner.x, maxCorner.x);

				}
			}
		}

		sandbox->playerSpeed = sandbox->playerSpeed +
			playerAcceleration * GlobalDeltaTime;
		sandbox->playerP.offset = oldPlayerPos.offset + playerDelta * tMin;
		sandbox->playerP = RecanonicalizePosition(tilemap, sandbox->playerP);

		//sandbox->playerSpeed = Reflect(sandbox->playerSpeed,  n);

		ChunkPosition chunkPos = GetChunkPosition(tilemap, sandbox->playerP.tileX,
												  sandbox->playerP.tileY);
		DrawWorld(tilemap, sandbox->renderGroup, assetManager,
				  sandbox->playerP);

		f32 pX = (chunkPos.tileInChunkX * tilemap->tileSizeInUnits +
				  sandbox->playerP.offset.x) * tilemap->unitsToRaw;
		f32 pY = (chunkPos.tileInChunkY * tilemap->tileSizeInUnits +
				  sandbox->playerP.offset.y) * tilemap->unitsToRaw;

		pY = (tilemap->chunkSizeInTiles * tilemap->tileSizeRaw) - pY;
			
		DrawDebugCube(sandbox->renderGroup,
					  assetManager,
					  V3(pX, 3.0f, pY),
					  tilemap->tileSizeRaw * 0.5f, V3(1.0f, 0.0f, 0.0f));
		sandbox->camera.target = V3(pX, 0, pY);
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

		UpdateCamera(&sandbox->camera, inputManager);
		RenderGroupSetCamera(sandbox->renderGroup,
							 sandbox->camera.front, sandbox->camera.pos);
		
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

	
	void UpdateCamera(AnnoCamera* cam, InputManager* inputManager)
	{
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

		if (InputMouseButtonIsDown(inputManager, MouseButton::Right))
		{
			v2 mousePos = InputGetMouseFrameOffset(inputManager);
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
		
		i32 frameScrollOffset = InputGetFrameScrollOffset(inputManager);
		cam->targetDistance -= frameScrollOffset * scrollSpeed;

		if (cam->targetDistance < 2.0f)
		{
			cam->targetDistance = 2.0f;
		} else if (cam->targetDistance > 120.0f)
		{
			cam->targetDistance = 120.0f;
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
		
		DEBUG_OVERLAY_PUSH_VAR("cam front", cam->front);
	}

}
