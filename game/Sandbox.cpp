#include "Sandbox.h"
// TODO: This is temporary for rand()
#include <stdlib.h>

#include "World.cpp"
#include "Camera.cpp"


namespace AB
{	
	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  GameState* gameState,
			  AssetManager* assetManager)
	{
		gameState->renderGroup = AllocateRenderGroup(arena, MEGABYTES(300),
													 128000, 32);
#if 0
		gameState->renderGroup->projectionMatrix = PerspectiveRH(45.0f,
																 16.0f / 9.0f,
																 0.1f,
																 300.0f);
#endif
		gameState->dirLight.ambient = V3(0.05f);
		gameState->dirLight.diffuse = V3(1.1f);
		gameState->dirLightOffset = V3(-10, 38, 17);
		
		BeginTemporaryMemory(tempArena);
#if 0
		gameState->mansionMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/mansion/mansion.aab");
		gameState->planeMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena, tempArena,
							   "../assets/Plane.aab");
#endif
		
		//AB_CORE_ASSERT(gameState->mansionMeshHandle != ASSET_INVALID_HANDLE);
		EndTemporaryMemory(tempArena);

		gameState->camera = {};
		gameState->camera.longSmoothness = 0.3f;
		gameState->camera.latSmoothness = 0.3f;
		gameState->camera.distSmoothness = 0.3f;
		gameState->camera.pos = V3(0.0f, 0.0f, -1.0f);
		gameState->camera.front = V3(0.0, 0.0f, 1.0f);
		gameState->camera.fov = 45.0f;
		gameState->camera.aspectRatio = 16.0f / 9.0f;
		gameState->camera.nearPlane = 0.1f;
		gameState->camera.farPlane = 200.0f;
		
		gameState->camera.targetWorldPos.chunkX = 0;
		gameState->camera.targetWorldPos.chunkY = 0;
		gameState->camera.targetWorldPos.offset.x = 1.0f;
		gameState->camera.targetWorldPos.offset.y = 1.0f;


		//SubscribeKeyboardEvents(&gameState->camera, inputManager,
		//						&gameState->inputState);

		gameState->gamma = 2.2f;
		gameState->exposure = 1.0f;

		gameState->world = CreateWorld(arena);
		World* world = gameState->world;
	   
		f32 r = 0.2f;
		f32 g = 0.2f;
		f32 b = 0.2f;
		
		for (i32 y = -4;
			 y < (i32)(world->chunkCountY) - 4;
			 y++)
		{
			for (i32 x = -4;
				 x < (i32)(world->chunkCountX) -4;
				 x++)
			{
				r = rand() % 11 / 10.0f;
				g = rand() % 11 / 10.0f;
				b = rand() % 11 / 10.0f;
				Chunk* chunk = GetChunk(world, x, y, arena);
				for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
				{
					for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
					{
#if 0
						if ((tileX == 0 || tileY == 0))
						{
							SetTerrainTile(chunk, tileX, tileY,
										   TERRAIN_TYPE_WATER);
							AddWallEntity(gameState, chunk,
										  V2(tileX * world->tileSizeInUnits,
											 tileY * world->tileSizeInUnits),
										  arena);
							
						}
						else
#endif
						{
							if ((x == -1 && tileX == 0) ||
								(x == world->chunkCountX - 15 &&
								 tileX == WORLD_CHUNK_DIM_TILES - 1) ||
								(y == -1 && tileY == 0) ||
								(y == world->chunkCountY - 15 &&
								 tileY == WORLD_CHUNK_DIM_TILES - 1))
							{
								SetTerrainTile(chunk, tileX, tileY,
											   TERRAIN_TYPE_CLIFF);
								AddWallEntity(world, chunk,
											  V2(tileX * world->tileSizeInUnits,
												 tileY * world->tileSizeInUnits),
											  arena);
								
							}
							else
							{
								SetTerrainTile(chunk, tileX, tileY,
											   TERRAIN_TYPE_GRASS);
							}
						}

					}
				}
			}
		}

		{
			Chunk* chunk = GetChunk(world, -3, -4, arena);
			for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
			{
				for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
				{
					SetTerrainTile(chunk, tileX, tileY, TERRAIN_TYPE_WATER);
				}
			}

		}

		Chunk* firstChunk = GetChunk(gameState->world, 0, 0);
		AB_ASSERT(firstChunk);

		gameState->entity = AddLowEntity(world, firstChunk,
										 ENTITY_TYPE_BODY, arena);
		LowEntity* e = GetLowEntity(world, gameState->entity);
		e->worldPos.chunkX = 0;
		e->worldPos.chunkY = 0;
		e->worldPos.offset = V2(10.0f, 10.0f);
		e->accelerationAmount = 20.0f;
		e->size = V2(1.5f, 3.0f);
		e->color = V3(1.0f, 0.0f, 0.0f);
		e->friction = 3.0f;
		e->velocity = V2(0.0f, 0.1f);
		
		gameState->entity1 = AddLowEntity(world, firstChunk,
										  ENTITY_TYPE_BODY, arena);
		LowEntity* e1 = GetLowEntity(world, gameState->entity1);
		e1->worldPos.chunkX = 0;
		e1->worldPos.chunkY = 0;
		e1->worldPos.offset = V2(0.0f, 0.0f);
		e1->accelerationAmount = 30.0f;
		e1->size = V2(1.2f, 0.5f);
		e1->color = V3(0.0f, 1.0f, 0.0f);
		e1->friction = 3.0f;
		e1->type = ENTITY_TYPE_BODY;

#if 1
		for (u32 i = 0; i < MOVING_ENTITIES_COUNT; i++)
		{
			u32 id = AddLowEntity(world, firstChunk,
								  ENTITY_TYPE_BODY, arena);
			gameState->movingEntities[i] = id; 
			LowEntity* e = GetLowEntity(world, id);
			if (i < 32)
			{
				e->worldPos.chunkX= 0;
				e->worldPos.chunkY= 0;
				e->worldPos.offset = V2((f32)(i + i * 2));
			}
			else if (i < 64)
			{
				e->worldPos.chunkX= 0;
				e->worldPos.chunkY= 0;
				e->worldPos.offset = V2((f32)(((i - 32) + (i - 32) * 2) + 4), (f32)((i - 32) + 18 + (i - 32) * 2));

			}
			else if (i < 96)
			{
				//e->worldPos.tileX = ((i - 64) + 18 + (i - 64) * 2) - 4;
				//e->worldPos.tileY = (i - 64) + 18 + (i - 64) * 2;
			}
			else
			{
				//e->worldPos.tileX = ((i - 96) + 18 + (i - 96) * 2) - 7;
				//e->worldPos.tileY = (i - 96) + 18 + (i - 96) * 2;				
			}
			
			e->accelerationAmount = 30.0f;

			f32 w = (f32)(rand() % 22 / 10.0f);
			f32 h = (f32)(rand() % 22 / 10.0f);

			e->size = V2(w, h);

			r = rand() % 11 / 10.0f;
			g = rand() % 11 / 10.0f;
			b = rand() % 11 / 10.0f;
			
			e->color = V3(r, g, b);
			e->friction = 0.0f;

			f32 x = (f32)(rand() % 11 * 5);
			f32 y = (f32)(rand() % 11 * 5);
			e->velocity = V2(x, y);
			
			
		}
#endif

	}
	
	void
	DrawEntity(World* world, RenderGroup* renderGroup,
			   AssetManager* assetManager, Entity entity, Camera* camera)
	{
		v2 camRelPos = GetCamRelPos(world, entity.low->worldPos,
									camera->targetWorldPos);
		v3 pos = V3(0.0f);
		pos.x = camRelPos.x * world->unitsToRaw;
		pos.z = camRelPos.y * world->unitsToRaw;
		pos.y = world->tileSizeRaw;

		v3 scale = V3(0.0f);
		scale.x = world->unitsToRaw * 0.5f * entity.low->size.x;
		scale.z = world->unitsToRaw * 0.5f * entity.low->size.y;
		scale.y = world->unitsToRaw * 0.5f;

		v3 color = entity.low->color;
		
		DrawDebugCube(renderGroup, assetManager, pos, scale, color);
	}

	WorldPosition
	EntityApplyMovement(World* world, LowEntity* entity, v2 delta)
	{
		WorldPosition resultPos = entity->worldPos;
		v2 colliderSize = entity->size;
		v2 iterationOffset = {};

		for (u32 pass = 0; pass < 4; pass++)
		{
			// TODO: Using AABB's to calculate chunks for checking
			// TODO: This is totally crappy code for collision
			// detection on chunk bounds
			// Needs to be rapaced with robust code.
			WorldPosition beginPos = resultPos;
			WorldPosition desiredPosP = ChangeWorldPosition(world,
															beginPos,
															delta + colliderSize);

			WorldPosition desiredPosN = ChangeWorldPosition(world,
															beginPos,
															delta - colliderSize);

			WorldPosition desiredPosC = ChangeWorldPosition(world,
															beginPos,
															delta);


			i32 minChunkX = MINIMUM(beginPos.chunkX,
									MINIMUM(desiredPosP.chunkX,
											desiredPosN.chunkX));
			i32 maxChunkX = MAXIMUM(beginPos.chunkX,
									MAXIMUM(desiredPosP.chunkX,
											desiredPosN.chunkX));
			i32 minChunkY = MINIMUM(beginPos.chunkY,
									MINIMUM(desiredPosP.chunkY,
											desiredPosN.chunkY));
			i32 maxChunkY = MAXIMUM(beginPos.chunkY,
									MAXIMUM(desiredPosP.chunkY,
											desiredPosN.chunkY));
			if (desiredPosC.offset.x > world->chunkSizeUnits / 2.0f)
			{
				maxChunkX++;
			}
			else
			{
				minChunkX--;
			}

			if (desiredPosC.offset.y > world->chunkSizeUnits / 2.0f)
			{
				maxChunkY++;
			}
			else
			{
				minChunkY--;
			}
			
			
			v2 wallNormal = V2(0.0f);
			f32 tMin = 1.0f;
			u32 hitEntityIndex = 0;
			WorldPosition targetPosition =
				ChangeWorldPosition(world, resultPos, delta);

			for (i32 chunkY = minChunkY; chunkY <= maxChunkY; chunkY++)
			{
				for (i32 chunkX = minChunkX; chunkX <= maxChunkX; chunkX++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY);
					// TODO: Handle world edges
					AB_ASSERT(chunk);
					// TODO: Make chunk high
					EntityBlock* block = &chunk->firstEntityBlock;
					do
					{
						for (u32 i = 0;
							 i < block->count;
							 i++)
						{
							u32 testEntityIndex = block->lowEntityIndices[i];
							LowEntity* testEntity =
								GetLowEntity(world, testEntityIndex);
							
							if (entity != testEntity)
							{
								v2 relOldPos = WorldPosDiff(world,
															resultPos,
															testEntity->worldPos);
								
								v2 minCorner = -0.5f * testEntity->size;
								v2 maxCorner = 0.5f * testEntity->size;
								//NOTE: Minkowski sum
								minCorner += colliderSize * -0.5f;
								maxCorner += colliderSize * 0.5f;

								if (TestWall(minCorner.x, relOldPos.x,
											 relOldPos.y, delta.x,
											 delta.y,
											 minCorner.y, maxCorner.y, &tMin))
								{
									wallNormal = V2(1.0f, 0.0f);
									hitEntityIndex = testEntityIndex;
								}
						
								if(TestWall(maxCorner.x, relOldPos.x,
											relOldPos.y, delta.x,
											delta.y,
											minCorner.y, maxCorner.y, &tMin))
								{
									wallNormal = V2(-1.0f, 0.0f);
									hitEntityIndex = testEntityIndex;
								}
								if(TestWall(minCorner.y, relOldPos.y,
											relOldPos.x, delta.y,
											delta.x,
											minCorner.x, maxCorner.x, &tMin))
								{
									wallNormal = V2(0.0f, 1.0f);
									hitEntityIndex = testEntityIndex;
								}
								if(TestWall(maxCorner.y, relOldPos.y,
											relOldPos.x, delta.y,
											delta.x,
											minCorner.x, maxCorner.x, &tMin))
								{
									wallNormal = V2(0.0f, -1.0f);
									hitEntityIndex = testEntityIndex;
								}
				
							}
						}
						block = block->nextBlock;
					} while (block);		
				}
			}
			
			resultPos = ChangeWorldPosition(world, resultPos, delta * tMin);
			
			if (hitEntityIndex)
			{
				delta = WorldPosDiff(world, targetPosition, resultPos);
				entity->velocity -=
					2.0f * Dot(entity->velocity, wallNormal) * wallNormal;
				delta *= 1.0f - tMin;
				delta -= Dot(delta, wallNormal) * wallNormal;				
			}
			else
			{
				break;
			}
		}
		return resultPos;
	}

	
	void
	MoveEntity(World* world, LowEntity* entity, v2 acceleration,
			   MemoryArena* arena)
	{
		f32 speed = entity->accelerationAmount;
		acceleration *= speed;

		f32 friction = entity->friction;
		acceleration = acceleration - entity->velocity * friction;

		v2 movementDelta;
		movementDelta = 0.5f * acceleration *
			Square(GlobalGameDeltaTime) + 
			entity->velocity *
			GlobalGameDeltaTime;

		auto newPos = EntityApplyMovement(world, entity,  movementDelta);

		entity->velocity += acceleration * GlobalGameDeltaTime;

		// TODO: Move it to more appropriate place maybe.
		// This stuff should happens once at the end of update loop _maybe_
		
		ChangeEntityPosition(world, entity, newPos, arena);
	}

	inline i32
	SafeAddI32I32(i32 a, i32 b);
	

	inline i32
	SafeSubI32I32(i32 a, i32 b)
	{
		i32 result = 0;
		if (b < 0)
		{
			result = SafeAddI32I32(a, -b);
		}
		else
		{
			result = a - b;
			if (result > a)
			{
				result = AB_INT32_MIN;
			}
		}
		
		return result;
	}

	inline i32
	SafeAddI32I32(i32 a, i32 b)
	{
		i32 result;
		if (b < 0)
		{
			result = SafeSubI32I32(a, -b);
		}
		else
		{
			result = a + b;
			if (result < a)
			{
				result = AB_INT32_MAX;
			}
		}

		return result;
	}

	void
	MoveCamera(GameState* gameState, Camera* camera, World* world,
			   AssetManager* assetManager)
	{
		v2 entityFrameOffset = -MoveCameraTarget(&gameState->camera, world);
		UpdateCamera(camera, gameState->renderGroup, world);

		i32 highAreaChunkSpanX = 1;
		i32 highAreaChunkSpanY = 1;

		i32 minChunkX = SafeSubI32I32(camera->targetWorldPos.chunkX,
									  highAreaChunkSpanX);
		i32 minChunkY = SafeSubI32I32(camera->targetWorldPos.chunkY,
									  highAreaChunkSpanY);
		i32 maxChunkX = SafeAddI32I32(camera->targetWorldPos.chunkX,
									  highAreaChunkSpanX);
		i32 maxChunkY = SafeAddI32I32(camera->targetWorldPos.chunkY,
									  highAreaChunkSpanY); 

		DEBUG_OVERLAY_TRACE_VAR(camera->targetWorldPos.chunkX);
		DEBUG_OVERLAY_TRACE_VAR(camera->targetWorldPos.chunkY);
		DEBUG_OVERLAY_TRACE_VAR(camera->targetWorldPos.offset.x);
		DEBUG_OVERLAY_TRACE_VAR(camera->targetWorldPos.offset.y);

		for (u32 index = 1; index <= world->highEntityCount;)
		{
			Entity entity = GetEntityFromHighIndex(world, index);
			i32 entityChunkX = entity.low->worldPos.chunkX;
			i32 entityChunkY = entity.low->worldPos.chunkY;
			
			if ((entityChunkX < minChunkX) ||
				(entityChunkX > maxChunkX) ||
				(entityChunkY < minChunkY) ||
				(entityChunkY > maxChunkY))
			{
				// TODO: Instead of travercing entities
				// keep list of high chunks and check _if chunk is outside_
				// and evict whole chunk
				Chunk* chunk = GetChunk(world, entityChunkX, entityChunkY);
				if (chunk)
				{
					chunk->high = false;
					SetEntityToLow(world, &gameState->camera, index);
				}
				else
				{
					index++;
				}
			}
			else
			{
				index++;
			}
		}

		for (i32 chunkY = minChunkY; chunkY <= maxChunkY; chunkY++)
		{
			for (i32 chunkX = minChunkX; chunkX <= maxChunkX; chunkX++)
			{
				Chunk* chunk = GetChunk(world, chunkX, chunkY);
				if (chunk)
				{
					if (!(chunk->high)) 
					{
						chunk->high = true;

						EntityBlock* block = &chunk->firstEntityBlock;
						for (u32 i = 0; i < block->count; i++)
						{
							SetEntityToHigh(world, &gameState->camera,
											block->lowEntityIndices[i]);
						}
						while (block->nextBlock)
						{
							block = block->nextBlock;
							for (u32 i = 0; i < block->count; i++)
							{
								SetEntityToHigh(world, &gameState->camera, block->lowEntityIndices[i]);
							}					
						}
					}
				}
			}
		}

		v2 minLine = WorldPosDiff(world, {minChunkX, minChunkY, 0, 0},
								  camera->targetWorldPos);

		v2 maxLine = WorldPosDiff(world,
								  {maxChunkX, maxChunkY,
										  world->chunkSizeUnits, world->chunkSizeUnits},
								  camera->targetWorldPos);

		DrawAlignedBoxOutline(gameState->renderGroup, assetManager,
							  V3(minLine.x, 3.0f, minLine.y) * world->unitsToRaw,
							  V3(maxLine.x, 10.0f, maxLine.y) * world->unitsToRaw,
							  V3(0.8, 0.0, 0.0), 2.0f);

		DEBUG_OVERLAY_TRACE_VAR(world->lowEntityCount);
		DEBUG_OVERLAY_TRACE_VAR(world->highEntityCount);
	}

	void Render(MemoryArena* arena,
				GameState* gameState,
				AssetManager* assetManager,
				Renderer* renderer)
	{
		DEBUG_OVERLAY_SLIDER(g_Platform->gameSpeed, 0.0f, 10.0f);
		World* world = gameState->world;
		Camera* camera = &gameState->camera;

		DEBUG_OVERLAY_TRACE_VAR(world->nonResidentEntityBlocksCount);
		DEBUG_OVERLAY_TRACE_VAR(world->freeEntityBlockCount);
		{

			MoveCamera(gameState, camera, world, assetManager);
			
			for (u32 index = 1; index <= world->highEntityCount; index++)
			{
				Entity entity = GetEntityFromHighIndex(world, index);

				if (entity.low->type == ENTITY_TYPE_BODY)
				{
					MoveEntity(world, entity.low , V2(0.0f) ,arena);
				}
				
				DrawEntity(world, gameState->renderGroup,
						   assetManager, entity, camera);
			}
		}

		//SetEntityToHigh(gameState, gameState->entity);
		//SetEntityToHigh(gameState, gameState->entity1);
		Entity entity = GetEntityFromLowIndex(world, gameState->entity);
		
		Entity entity1 = GetEntityFromLowIndex(world, gameState->entity1);

		if (entity.high)
		{ // Enitity 0 movement code
			v3 _frontDir = V3(camera->front.x, 0.0f, camera->front.z);
			v3 _rightDir = Cross(V3(0.0f, 1.0f, 0.0f), _frontDir);
			_rightDir = Normalize(_rightDir);

			v2 frontDir = V2(_frontDir.x, _frontDir.z);
			v2 rightDir = V2(_rightDir.x, _rightDir.z);

			v2 acceleration = {};
			if (GlobalInput.keys[KEY_UP].pressedNow)
			{
				acceleration += frontDir;
			}
			if (GlobalInput.keys[KEY_DOWN].pressedNow)
			{
				acceleration -= frontDir;
			}
			if (GlobalInput.keys[KEY_LEFT].pressedNow)
			{
				acceleration -= rightDir;
			}
			if (GlobalInput.keys[KEY_RIGHT].pressedNow)
			{
				acceleration += rightDir;
			}
		
			acceleration = Normalize(acceleration);

			MoveEntity(world, entity.low, acceleration,arena);
			DEBUG_OVERLAY_TRACE_VAR(entity.low->worldPos.chunkX);
			DEBUG_OVERLAY_TRACE_VAR(entity.low->worldPos.chunkY);
			DEBUG_OVERLAY_TRACE_VAR(entity.low->worldPos.offset.x);
			DEBUG_OVERLAY_TRACE_VAR(entity.low->worldPos.offset.y);
		}

		if (entity1.high)
		{ // Enitity 1 movement code
			v3 _frontDir = V3(camera->front.x, 0.0f, camera->front.z);
			v3 _rightDir = Cross(V3(0.0f, 1.0f, 0.0f), _frontDir);
			_rightDir = Normalize(_rightDir);

			v2 frontDir = V2(_frontDir.x, _frontDir.z);
			v2 rightDir = V2(_rightDir.x, _rightDir.z);

			v2 acceleration = {};
			if (GlobalInput.keys[KEY_NUMPAD8].pressedNow)
			{
				acceleration += frontDir;
			}
			if (GlobalInput.keys[KEY_NUMPAD5].pressedNow)
			{
				acceleration -= frontDir;
			}
			if (GlobalInput.keys[KEY_NUMPAD4].pressedNow)
			{
				acceleration -= rightDir;
			}
			if (GlobalInput.keys[KEY_NUMPAD6].pressedNow)
			{
				acceleration += rightDir;
			}
		
			acceleration = Normalize(acceleration);
			MoveEntity(world, entity1.low, acceleration, arena);
			DEBUG_OVERLAY_TRACE_VAR(entity1.low->worldPos.chunkX);
			DEBUG_OVERLAY_TRACE_VAR(entity1.low->worldPos.chunkY);
			DEBUG_OVERLAY_TRACE_VAR(entity1.low->worldPos.offset.x);
			DEBUG_OVERLAY_TRACE_VAR(entity1.low->worldPos.offset.y);
		}
		
		
		DrawWorldInstanced(world, gameState->renderGroup, assetManager,
						   gameState->camera.targetWorldPos);

		//gameState->camera.target = V3(pX, 0, pY);
		//gameState->dirLight.target = V3(pX, 0, pY);
		

#if 0
		DEBUG_OVERLAY_PUSH_SLIDER("Gamma", &gameState->gamma, 0.0f, 3.0f);
		DEBUG_OVERLAY_PUSH_VAR("Gamma", gameState->gamma);

		DEBUG_OVERLAY_PUSH_SLIDER("Exposure", &gameState->exposure, 0.0f, 3.0f);
		DEBUG_OVERLAY_PUSH_VAR("Exposure", gameState->exposure);
#endif
		renderer->cc.gamma = gameState->gamma;
		renderer->cc.exposure = gameState->exposure;
		
		RenderCommandDrawMesh planeCommand = {};
		planeCommand.meshHandle = gameState->mansionMeshHandle;
		planeCommand.transform.worldMatrix = Identity4();
		planeCommand.blendMode = BLEND_MODE_OPAQUE;

		f32 ka = gameState->dirLight.ambient.r;
		f32 kd = gameState->dirLight.diffuse.r;
#if 0
		DEBUG_OVERLAY_PUSH_SLIDER("Ka", &ka, 0.0f, 1.0f);
		DEBUG_OVERLAY_PUSH_VAR("Ka", ka);

		DEBUG_OVERLAY_PUSH_SLIDER("Kd", &kd, 0.0f, 10.0f);
		DEBUG_OVERLAY_PUSH_VAR("Kd", kd);

		DEBUG_OVERLAY_TRACE_VAR(gameState->dirLight.from);
		DEBUG_OVERLAY_TRACE_VAR(gameState->dirLight.target);
		DEBUG_OVERLAY_TRACE_VAR(gameState->dirLightOffset);
		DEBUG_OVERLAY_PUSH_SLIDER("offset", &gameState->dirLightOffset, -50, 50);
#endif
		gameState->dirLight.from = AddV3V3(gameState->dirLight.target,
										 gameState->dirLightOffset);
		gameState->dirLight.ambient = V3(ka);
		gameState->dirLight.diffuse = V3(kd);
		

		RenderCommandSetDirLight dirLightCommand = {};
		dirLightCommand.light = gameState->dirLight;

		RenderGroupPushCommand(gameState->renderGroup,
							   assetManager,
							   RENDER_COMMAND_SET_DIR_LIGHT,
							   (void*)(&dirLightCommand));

		RendererRender(renderer, assetManager, gameState->renderGroup);
		RenderGroupResetQueue(gameState->renderGroup);
	}

}


// NOTE: Retired
#if 0
#define TEST
#if !defined(TEST)
void DrawWorld(World* world, RenderGroup* renderGroup,
			   AssetManager* assetManager, WorldPosition player)
{
	Chunk* chunks[9];
	SetArray(Chunk*, 9, &chunks, 0);
	ChunkPosition cPos = GetChunkPosition(world, player.tileX, player.tileY);
	chunks[0] = GetChunk(world, cPos.chunkX - 1, cPos.chunkY + 1);
	chunks[1] = GetChunk(world, cPos.chunkX, cPos.chunkY + 1);
	chunks[2] = GetChunk(world, cPos.chunkX + 1, cPos.chunkY + 1);
	chunks[3] = GetChunk(world, cPos.chunkX - 1, cPos.chunkY);
	chunks[4] = GetChunk(world, cPos.chunkX, cPos.chunkY);
	chunks[5] = GetChunk(world, cPos.chunkX + 1, cPos.chunkY);
	chunks[6] = GetChunk(world, cPos.chunkX - 1, cPos.chunkY - 1);
	chunks[7] = GetChunk(world, cPos.chunkX, cPos.chunkY - 1);
	chunks[8] = GetChunk(world, cPos.chunkX + 1, cPos.chunkY - 1);

	for (u32 chunkY = 0; chunkY < 3; chunkY++)
	{
		for (u32 chunkX = 0; chunkX < 3; chunkX++)
		{
			for (u32 row = 0; row < world->chunkSizeInTiles; row++)
			{
				for (u32 col = 0; col < world->chunkSizeInTiles; col++)
				{
					u32 tileValue = GetTileValueInChunk(world,
														chunks[chunkY * 3 + chunkX],
														col, row);
					if (tileValue)
					{
						v3 color;
						f32 yOffset = 0.0f;
						if (GetTileValueInChunk(world,
												chunks[chunkY * 3 + chunkX],
												col, row) == 3)
						{
							yOffset = 2.0f;
						}
						else
						{
							yOffset = 0.0f;
						}
						color = GetTileColor(world,
											 chunks[chunkY * 3 + chunkX],
											 col, row);
						ChunkPosition playerCPos =
							GetChunkPosition(world,
											 player.tileX, player.tileY);
						if (row == playerCPos.tileInChunkY &&
							col == playerCPos.tileInChunkX)// &&
							//chunkX == playerCPos.chunkX &&
							//chunkY == playerCPos.chunkY)
						{
							color = V3(0.0f);
						}
			

						f32	chunkOffsetX = chunkX * world->chunkSizeInTiles *
							world->tileSizeRaw - world->chunkSizeInTiles *
							world->tileSizeRaw;
						f32 chunkOffsetY = chunkY * world->chunkSizeInTiles *
							world->tileSizeRaw - world->chunkSizeInTiles *
							world->tileSizeRaw;
					
						f32 zOffset = row *	(f32)world->tileSizeRaw -
							(world->tileRadiusInUnits * world->unitsToRaw);
j
						f32 xOffset = col * (f32)world->tileSizeRaw -
							(world->tileRadiusInUnits * world->unitsToRaw);

						f32 zInvBias = world->chunkSizeInTiles *
							(f32)world->tileSizeRaw; 

						v3 position = V3(xOffset + chunkOffsetX,
										 yOffset,
										 (-zOffset + zInvBias) + chunkOffsetY);

						DrawDebugCube(renderGroup,
									  assetManager,
									  position, world->tileSizeRaw * 0.5f, color);
					}
				}
			}
		}
	}
}
#else
void DrawWorld(const World* world, RenderGroup* renderGroup,
			   AssetManager* assetManager, WorldPosition player,
			   u32 negChunkCountX, u32 negChunkCountY,
			   u32 posChunkCountX, u32 posChunkCountY)
{
	ChunkPosition pChunkPos = GetChunkPosition(world, player.tileX,
											   player.tileY); 

	u32 chunkCountX = negChunkCountX + posChunkCountX + 1;
	u32 chunkCountY = negChunkCountY + posChunkCountY + 1;

	v2 origin = V2(0.0f);
	origin.x = -1.0f * negChunkCountX * world->chunkSizeInTiles
		* world->tileSizeInUnits
		- pChunkPos.tileInChunkX * world->tileSizeInUnits
		- player.offset.x;
	origin.y = -1.0f * negChunkCountY * world->chunkSizeInTiles
		* world->tileSizeInUnits
		+ (world->chunkSizeInTiles - pChunkPos.tileInChunkY)
		* world->tileSizeInUnits - ( player.offset.y);

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
			Chunk* chunk = GetChunk(world,
									chunkX, chunkY);

			f32 chunkOffsetX = origin.x + (chunkX - beginChunkX + biasX) *
				world->chunkSizeInTiles * world->tileSizeInUnits;
			f32 chunkOffsetY = origin.y + (chunkY - beginChunkY + biasY) *
				world->chunkSizeInTiles * world->tileSizeInUnits;

			for (u32 row = 0; row < world->chunkSizeInTiles; row++)
			{
				for (u32 col = 0; col < world->chunkSizeInTiles; col++)
				{
					u32 tileValue = GetTileValueInChunk(world,
														chunk,
														col, row);
					if (tileValue)
					{
						v3 color;
						f32 yOffset = 0.0f;
						if (GetTileValueInChunk(world,
												chunk,
												col, row) == 3)
						{
							yOffset = 2.0f;
						}
						else
						{
							yOffset = 0.0f;
						}
						color = GetTileColor(world,
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
							(f32)world->tileSizeInUnits -
							world->tileRadiusInUnits;

						f32 xOffset = col *
							(f32)world->tileSizeInUnits -
							world->tileRadiusInUnits;

						f32 zInvBias = world->chunkSizeInTiles *
							(f32)world->tileSizeInUnits; 

						v3 position = V3(0.0f);
						position.x = (xOffset + chunkOffsetX)
							* world->unitsToRaw;
						position.y = yOffset;
						position.z = (-zOffset + zInvBias - chunkOffsetY)
							* world->unitsToRaw;

						DrawDebugCube(renderGroup,
									  assetManager,
									  position, world->tileSizeRaw * 0.5f, color);
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
			for (u32 row = 0; row < world->chunkSizeInTiles; row++)
			{
				for (u32 col = 0; col < world->chunkSizeInTiles; col++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY);
					u32 tileValue = GetTileValueInChunk(world,
														chunk,
														col, row);
					if (tileValue)
					{
						v3 color;
						f32 yOffset = 0.0f;
						if (GetTileValueInChunk(world,
												chunk,
												col, row) == 3)
						{
							yOffset = 2.0f;
						}
						else
						{
							yOffset = 0.0f;
						}
						color = GetTileColor(world,
											 chunk,
											 col, row);
						ChunkPosition playerCPos =
							GetChunkPosition(world,
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
		m3x3 camRot = M3x3(gameState->renderGroup->camera.lookAt);
		GenFrustumVertices(&camRot, gameState->camera.pos, 0.1f, 200.0f, 45.0f,
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

		v3 relCamPos = gameState->camera.pos - gameState->camera.target;
		
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


	b32 RaycastToGround(const World* tilemap, v3 from, v3 dir, 
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
    void DrawTile(World* tilemap, RenderGroup* renderGroup,
				  AssetManager* assetManager, WorldPosition origin,
				  WorldPosition tile)
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


void DrawWorld(World* tilemap, RenderGroup* renderGroup,
			   AssetManager* assetManager, WorldPosition origin,
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
					 WorldPosition{x, y});

		}
	}

}
	void DrawWorldInstanced(World* tilemap, RenderGroup* renderGroup,
							AssetManager* assetManager, WorldPosition origin,
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
								  WorldPosition{x, y}, nullptr);

			}
		}

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING,
							   nullptr);
	}

	void DrawEntity(World* tilemap,
					RenderGroup* renderGroup,
					AssetManager* assetManager,
					WorldPosition entityPos,
					WorldPosition origin,
					v3 color,
					f32 scale)
	{
		i32 relTileOffsetX = entityPos.tileX - origin.tileX;
		f32 relOffsetX = entityPos.offset.x - origin.offset.x;
			//+ tilemap->tileRadiusInUnits;
		i32 relTileOffsetY = entityPos.tileY - origin.tileY;
		f32 relOffsetY = entityPos.offset.y - origin.offset.y;
			//+ tilemap->tileRadiusInUnits;
		
		f32 pX = relTileOffsetX * tilemap->tileSizeInUnits + relOffsetX;
		f32 pY = relTileOffsetY * tilemap->tileSizeInUnits + relOffsetY;

		pX *= tilemap->unitsToRaw;
		pY *= tilemap->unitsToRaw;
		//pY = (tilemap->chunkSizeInTiles * tilemap->tileSizeRaw) - pY;
			
		DrawDebugCube(renderGroup,
					  assetManager,
					  V3(pX, 3.0f, pY),
					  V3(tilemap->tileSizeRaw * 0.5f * scale),
					  color);

	}

	static void
	ChangeEntityResidence(GameState* gameState, u32 index,
						  EntityResidence residence)
	{
		if (residence == ENTITY_RESIDENCE_HIGH)
		{
			if (gameState->entityResidence[index] != ENTITY_RESIDENCE_HIGH)
			{
				HighEntity* high = &gameState->highEntities[index];
				LowEntity* low = &gameState->lowEntities[index];

				high->pos = WorldPosDiff(&gameState->world->tilemap,
										   &low->tilemapPos,
										   &gameState->camera.targetWorldPos);
				high->velocity = V2(0.0f);
			}
		}
		else if (residence == ENTITY_RESIDENCE_LOW)
		{
			if (gameState->entityResidence[index] == ENTITY_RESIDENCE_HIGH)
			{
				HighEntity* high = &gameState->highEntities[index];
				LowEntity* low = &gameState->lowEntities[index];

				low->tilemapPos =
					MapToTileSpace(&gameState->world->tilemap,
								   gameState->camera.targetWorldPos,
								   high->pos);

			}
		}
		gameState->entityResidence[index] = residence;		
	}

inline Entity
GetEntity(GameState* gameState, u32 index, EntityResidence residence)
{
	Entity entity = {};

	if (index > 0 && index <= gameState->entityCount)
	{
		ChangeEntityResidence(gameState, index, residence);
		entity.residence = residence;
		entity.low = &gameState->lowEntities[index];
		entity.high = &gameState->highEntities[index];
	}

	return entity;
}
	
		
#endif
