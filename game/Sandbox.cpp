#include "Sandbox.h"

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

	inline void RecanonicalizeCoord(f32 tileSizeUnits,
									i32 tileCount,
									i32 tilemapCount,
									i32* tilemapCoord,
									i32* tileCoord,
									f32* coord)
	{
		i32 tileOffset = Floor(*coord / tileSizeUnits);
		f32 recanCoord = *coord - (f32)tileOffset * tileSizeUnits;
		i32 tilemapOffset = Floor((f32)(*tileCoord + tileOffset) / (f32)tileCount);
		i32 recanTileCoord = (*tileCoord + tileOffset) % tileCount;
		i32 recanTilemap = tilemapOffset + *tilemapCoord;

		// NOTE: Maybe there is a way to do it without is at==statement
		if (recanTileCoord < 0)
		{
			recanTileCoord = tileCount +  recanTileCoord;
		}

		if (recanCoord < 0)
		{
			recanCoord = tileSizeUnits - recanCoord;
		}

		*tilemapCoord = recanTilemap;
		*tileCoord = recanTileCoord;
		*coord = recanCoord;
	}

	inline WorldPosition RecanonicalizePosition(World* world,
													WorldPosition pos)
	{
		RecanonicalizeCoord(world->tileSizeInUnits, world->tilemapWidth,
							world->width, &pos.tilemapX, &pos.tileX, &pos.x);
		RecanonicalizeCoord(world->tileSizeInUnits, world->tilemapHeight,
							world->height, &pos.tilemapY, &pos.tileY, &pos.y);

		AB_ASSERT(pos.tilemapX >= 0);
		AB_ASSERT(pos.tilemapY >= 0);
		AB_ASSERT(pos.tilemapX < world->width);
		AB_ASSERT(pos.tilemapY < world->height);
		AB_ASSERT(pos.tileX >= 0);
		AB_ASSERT(pos.tileY >= 0);
		AB_ASSERT(pos.tileX < world->tilemapWidth);
		AB_ASSERT(pos.tileY < world->tilemapHeight);
		AB_ASSERT(pos.x >= 0);
		AB_ASSERT(pos.y >= 0);
		AB_ASSERT(pos.x < world->tileSizeInUnits);
		AB_ASSERT(pos.y < world->tileSizeInUnits);

		return pos;
	}

	inline Tilemap* GetTilemap(World* world, i32 x, i32 y)
	{
		Tilemap* tilemap = nullptr;
		if (x >= 0 && x < world->width &&
			y >= 0 && y < world->height)
		{
			tilemap =  &world->tilemaps[y * world->width + x];			
		}
		return tilemap;
	}

	inline u32 GetTileValueUnchecked(World* world, Tilemap* tilemap,
									 u32 tileX, u32 tileY)
	{
		return tilemap->tiles[tileY * world->tilemapWidth + tileX];
	}

	b32 TestWorldPoint(World* world, WorldPosition p/*i32 tilemapX, i32 tilemapY, f32 x, f32 y*/)
	{
		Tilemap* tilemap = GetTilemap(world, p.tilemapX, p.tilemapY);
		u32 tileValue = GetTileValueUnchecked(world, tilemap, p.tileX, p.tileY);
		return tileValue == 0;
	}

	void DrawTilemap(World* world, Tilemap* tilemap, RenderGroup* renderGroup,
					 AssetManager* assetManager, i32 playerTileX, i32 playerTileY)
	{

		for (i32 row = 0; row < world->tilemapHeight; row++)
		{
			for (i32 col = 0; col < world->tilemapWidth; col++)
			{
				v3 color;
				f32 yOffset;
				if (GetTileValueUnchecked(world, tilemap, col, row) == 0)
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

				f32 zOffset = world->height * (f32)world->tileSizeRaw  - row * (f32)world->tileSizeRaw;

				v3 position = V3(col * (f32)world->tileSizeRaw,
								 yOffset,
								 zOffset);

				DrawDebugCube(renderGroup,
							  assetManager,
							  position, world->tileSizeRaw * 0.5f, color);
			}
		}
	}
	
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
		sandbox->renderGroup = AllocateRenderGroup(arena, MEGABYTES(1),
												   1024, 32);
		sandbox->renderGroup->projectionMatrix = PerspectiveRH(45.0f,
															   16.0f / 9.0f,
															   0.1f,
															   100.0f);

		BeginTemporaryMemory(tempArena);
		sandbox->mansionMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/mansion/mansion.aab");
		sandbox->planeMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena, tempArena,
							   "../assets/Plane.aab");
		
		AB_CORE_ASSERT(sandbox->mansionMeshHandle != ASSET_INVALID_HANDLE);
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

		sandbox->playerP.tilemapX = 0;
		sandbox->playerP.tilemapY = 0;
		sandbox->playerP.tileX = 2;
		sandbox->playerP.tileY = 2;
		sandbox->playerP.x = 1.0f;
		sandbox->playerP.y = 1.0f;
	}

	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer,
				InputManager* inputManager)
	{
		const u32 TILEMAP_WIDTH = 16;
		const u32 TILEMAP_HEIGHT = 9;
	
		u32 tileMap00[TILEMAP_HEIGHT][TILEMAP_WIDTH] =
			{
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
				{1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			};

		u32 tileMap10[TILEMAP_HEIGHT][TILEMAP_WIDTH] =
			{
				{1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
				{1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			};

		u32 tileMap01[TILEMAP_HEIGHT][TILEMAP_WIDTH] =
			{
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
				{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
				{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			};

		u32 tileMap11[TILEMAP_HEIGHT][TILEMAP_WIDTH] =
			{
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
				{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
				{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			};

		World world;
		world.width = 2;
		world.height = 2;
		world.tilemapWidth = 16;
		world.tilemapHeight = 9;
		world.tileSizeRaw = 3.0f;
		world.tileSizeInUnits = 1.0f;
		world.toWorldUnits = world.tileSizeInUnits / world.tileSizeRaw;
		world.unitsToRaw = world.tileSizeRaw / world.tileSizeInUnits;
	
		Tilemap tilemaps[2][2];
		world.tilemaps = (Tilemap*)tilemaps;

		tilemaps[0][0].tiles = (u32*)tileMap00;
		tilemaps[0][1].tiles = (u32*)tileMap01;
		tilemaps[1][0].tiles = (u32*)tileMap10;
		tilemaps[1][1].tiles = (u32*)tileMap11;

		f32 newPlayerX = sandbox->playerP.x;
		f32 newPlayerY = sandbox->playerP.y;
		f32 playerSpeed = 3.0f;
		if (InputKeyIsDown(inputManager, KeyboardKey::W))
		{
			newPlayerY += playerSpeed * GlobalDeltaTime;
		}
		if (InputKeyIsDown(inputManager, KeyboardKey::S))
		{
			newPlayerY -= playerSpeed * GlobalDeltaTime;
		}
		if (InputKeyIsDown(inputManager, KeyboardKey::A))
		{
			newPlayerX -= playerSpeed * GlobalDeltaTime;
		}
		if (InputKeyIsDown(inputManager, KeyboardKey::D))
		{
			newPlayerX += playerSpeed * GlobalDeltaTime;
		}

		WorldPosition test1 = sandbox->playerP;
		test1.x = newPlayerX;
		test1.y = newPlayerY;
		test1 = RecanonicalizePosition(&world, test1);
		WorldPosition test2 = sandbox->playerP;
		test2.x = newPlayerX + world.tileSizeInUnits;
		test2.y = newPlayerY;
		test2 = RecanonicalizePosition(&world, test2);
		WorldPosition test3 = sandbox->playerP;
		test3.x = newPlayerX;
		test3.y = newPlayerY + world.tileSizeInUnits;
		test3 = RecanonicalizePosition(&world, test3);
		WorldPosition test4 = sandbox->playerP;
		test4.x = newPlayerX + world.tileSizeInUnits;
		test4.y = newPlayerY + world.tileSizeInUnits;
		test4 = RecanonicalizePosition(&world, test4);
		
		if (TestWorldPoint(&world, test1) &&
			TestWorldPoint(&world, test2) &&
			TestWorldPoint(&world, test3) &&
			TestWorldPoint(&world, test4))
	{
		sandbox->playerP.x = newPlayerX;
		sandbox->playerP.y = newPlayerY;
		sandbox->playerP = RecanonicalizePosition(&world, sandbox->playerP);
		DEBUG_OVERLAY_PUSH_VAR("tileY", sandbox->playerP.tileY);
		DEBUG_OVERLAY_PUSH_VAR("Y", sandbox->playerP.y);
	}

		Tilemap* playerTilemap = GetTilemap(&world, test1.tilemapX,
											test1.tilemapY);
		DrawTilemap(&world, playerTilemap, sandbox->renderGroup, assetManager,
					sandbox->playerP.tileX, sandbox->playerP.tileY);


		f32 playerPixelX = (sandbox->playerP.tileX * world.tileSizeInUnits
							+ sandbox->playerP.x) * world.unitsToRaw;
		f32 playerPixelY = world.height * (f32)world.tileSizeRaw - 
			(sandbox->playerP.tileY * world.tileSizeInUnits	+ sandbox->playerP.y) * world.unitsToRaw;
		DrawDebugCube(sandbox->renderGroup,
					  assetManager,
					  V3(playerPixelX, 1.0f, playerPixelY),
					  world.tileSizeRaw * 0.5f, V3(1.0f, 0.0f, 0.0f));
		
		
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

		DEBUG_OVERLAY_PUSH_SLIDER("Dir", &sandbox->dirLight.direction,
								  -1.0f, 1.0f);
		DEBUG_OVERLAY_PUSH_VAR("Dir", sandbox->dirLight.direction);
		sandbox->dirLight.direction = Normalize(sandbox->dirLight.direction);

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

		if (InputMouseButtonIsDown(inputManager, MouseButton::Middle))
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
