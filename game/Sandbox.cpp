#include "Sandbox.h"
// TODO: This is temporary for rand()
#include <stdlib.h>

#include "World.cpp"
#include "Camera.cpp"


namespace AB
{

	void DrawBlockHighlight(RenderGroup* renderGroup,
							AssetManager* assetManager,
							World* world,
							Camera* camera, TileWorldPos pos)
	{
		if (IsValid(pos))
		{
			v3 offset = V3(0.0f);
			Chunk* chunk = GetChunk(world, pos.chunkX, pos.chunkY);
			offset.x = (chunk->coordX - camera->targetWorldPos.chunkX) * WORLD_CHUNK_SIZE;
			offset.x -= camera->targetWorldPos.offset.x;
			offset.x += pos.tileX * WORLD_TILE_SIZE;
			offset.y = (chunk->coordY - camera->targetWorldPos.chunkY) * WORLD_CHUNK_SIZE;
			offset.y -= camera->targetWorldPos.offset.y;
			offset.y += pos.tileY * WORLD_TILE_SIZE;
			offset.z = pos.tileZ * WORLD_TILE_SIZE;
			offset.z -= WORLD_CHUNK_DIM_TILES * WORLD_TILE_SIZE;
			offset = FlipYZ(offset);

			v3 min = offset;
			v3 max = offset + V3(WORLD_TILE_SIZE);

			DrawAlignedBoxOutline(renderGroup, assetManager,
								  min, max, V3(1.0f, 0.0f, 0.0f), 1.0f);
		}
	}

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

		gameState->yDragSpeed = 30.0f;
		
		BeginTemporaryMemory(tempArena);
		gameState->treeFoliageHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/tree_foliage.aab");
		gameState->treeTrunkHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/tree_trunk.aab");

		u32 xAxisMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/gizmos/xAxis.aab");

		u32 yAxisMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/gizmos/yAxis.aab");

		u32 zAxisMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/gizmos/zAxis.aab");
		gameState->xAxisMesh = GetMesh(assetManager, xAxisMeshHandle);
		gameState->yAxisMesh = GetMesh(assetManager, yAxisMeshHandle);
		gameState->zAxisMesh = GetMesh(assetManager, zAxisMeshHandle);
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
			 y < 4;
			 y++)
		{
			for (i32 x = -4;
				 x < 4;
				 x++)
			{
				g = rand() % 11 / 10.0f;
				b = rand() % 11 / 10.0f;
				Chunk* chunk = GetChunk(world, x, y, arena);
				for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
				{
					for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
					{
						for (u32 tileZ = 0; tileZ < WORLD_CHUNK_DIM_TILES; tileZ++)
						{
							u32 bias = 0;
							if (tileZ == WORLD_CHUNK_DIM_TILES - 1)
							{
								bias = rand() % 2;									
							}

							TerrainTileData tile = GetTerrainTile(chunk,
																  tileX,
																  tileY,
																  tileZ - bias);
							//tile->height = rand() % 10 / 15.0f;
							if ((x == -4 && tileX == 0) ||
								(x == 3 &&
								 tileX == WORLD_CHUNK_DIM_TILES - 1) ||
								(y == -4 && tileY == 0) ||
								(y == 3 &&
								 tileY == WORLD_CHUNK_DIM_TILES - 1))
							{
								tile.type = TERRAIN_TYPE_CLIFF;
#if 0
								AddWallEntity(world, chunk,
											  V3(tileX * world->tileSizeInUnits,
												 tileY * world->tileSizeInUnits, 0.0f),
											  assetManager,
											  arena);
#endif
							}
							else
							{
								auto choise = rand() % 3;
								if (choise < 2)
								{
									tile.type = TERRAIN_TYPE_GRASS;									
								}
								else
								{
									tile.type = TERRAIN_TYPE_CLIFF;									
								}
							
							}
							SetTerrainTile(chunk, tileX, tileY, tileZ - bias, &tile);
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
					TerrainTileData tile = GetTerrainTile(chunk, tileX, tileY, 0);
					tile.type = TERRAIN_TYPE_WATER;
					SetTerrainTile(chunk, tileX, tileY, 0, &tile);
				}
			}

		}

		Chunk* firstChunk = GetChunk(gameState->world, 0, 0);
		AB_ASSERT(firstChunk);
#if 1
		MesherUpdateChunks(world->chunkMesher, world, tempArena);
#endif

		gameState->entity = AddLowEntity(world, firstChunk,
										 ENTITY_TYPE_BODY, arena);
		LowEntity* e = GetLowEntity(world, gameState->entity);
		e->worldPos.chunkX = 0;
		e->worldPos.chunkY = 0;
		e->worldPos.offset = V3(10.0f, 10.0f, 0.0f);
		e->accelerationAmount = 20.0f;
		e->size = 0.5f;
#if 0
		e->meshes[0] = assetManager->meshes + gameState->treeFoliageHandle;
		e->meshes[1] = assetManager->meshes + gameState->treeTrunkHandle;
		e->meshCount = 2;
#endif
		e->meshes[0] = GetMesh(assetManager, 0);
		e->meshCount = 1;
		
		e->color = V3(1.0f, 0.0f, 0.0f);
		e->friction = 3.0f;
		e->velocity = V2(0.0f, 0.1f);
		
		gameState->entity1 = AddLowEntity(world, firstChunk,
										  ENTITY_TYPE_BODY, arena);
		LowEntity* e1 = GetLowEntity(world, gameState->entity1);
		e1->worldPos.chunkX = 0;
		e1->worldPos.chunkY = 0;
		e1->worldPos.offset = V3(0.0f, 0.0f, 0.0f);
		e1->accelerationAmount = 30.0f;
		e1->size = 2.0f;
		e1->color = V3(0.0f, 1.0f, 0.0f);
		e1->friction = 3.0f;
		e1->type = ENTITY_TYPE_BODY;
		e1->meshes[0] = assetManager->meshes + gameState->treeFoliageHandle;
		e1->meshes[1] = assetManager->meshes + gameState->treeTrunkHandle;
		e1->meshCount = 2;


#if 0
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
	DrawEntity(GameState* gameState, AssetManager* assetManager,
			   Entity entity, bool drawBBox = false)
	{
		v3 camRelPos = GetCamRelPos(entity.low->worldPos,
									gameState->camera.targetWorldPos);
		v3 pos = V3(0.0f);
		pos.x = camRelPos.x;
		pos.z = camRelPos.y;

		v3 scale = V3(0.0f);
		scale.x = entity.low->size;
		scale.z = entity.low->size;
		scale.y = entity.low->size;
		// TODO: Mesh aligment, tilemap footprints n' stuff
		pos.y = camRelPos.z;

		//pos.x -= scale.x * 0.5f;
		//pos.z -= scale.z * 0.5f;
		v3 color = entity.low->color;
		bool selected = false;
		if (entity.low == gameState->selectedEntity)
		{
			selected = true;
		}
		for (u32 i = 0; i < entity.low->meshCount; i++)
		{
			Mesh* mesh = entity.low->meshes[i];
			DrawDebugMesh(gameState->renderGroup, assetManager, pos, scale,
						  mesh, selected);
			if (drawBBox || selected)
			{
				v3 worldPos = FlipYZ(entity.high->pos);
				v3 min = mesh->aabb.min * entity.low->size + worldPos;
				v3 max = mesh->aabb.max * entity.low->size + worldPos;

				DrawAlignedBoxOutline(gameState->renderGroup, assetManager,
									  min,
									  max,
									  V3(0.0f, 0.0f, 1.0f), 2.0f);
			}
			
		}
		if (selected)
		{
			scale.x = 1.0f;
			scale.z = 1.0f;
			scale.y = 1.0f;

			DrawDebugMesh(gameState->renderGroup, assetManager, pos,
						  scale,
						  gameState->xAxisMesh, false);

			DrawDebugMesh(gameState->renderGroup, assetManager, pos,
						  scale,
						  gameState->yAxisMesh, false);

			DrawDebugMesh(gameState->renderGroup, assetManager, pos,
						  scale,
						  gameState->zAxisMesh, false);
		}
	}

	WorldPosition
	DoCollisionDetection(World* world, LowEntity* entity, v2 delta)
	{
		WorldPosition resultPos = entity->worldPos;
		// TODO: bounding boxes
		v2 colliderSize = V2(entity->size);
		v2 iterationOffset = {};

		for (u32 pass = 0; pass < 4; pass++)
		{
			// TODO: Using AABB's to calculate chunks for checking
			// TODO: This is totally crappy code for collision
			// detection on chunk bounds
			// Needs to be rapaced with robust code.
			WorldPosition beginPos = resultPos;
			WorldPosition desiredPosP =
				OffsetWorldPos(beginPos, V3(delta + colliderSize, 0.0f));

			WorldPosition desiredPosN =
				OffsetWorldPos(beginPos, V3(delta - colliderSize, 0.0f));

			WorldPosition desiredPosC =
				OffsetWorldPos(beginPos, V3(delta, 0.0f));


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
			if (desiredPosC.offset.x > WORLD_CHUNK_SIZE / 2.0f)
			{
				maxChunkX++;
			}
			else
			{
				minChunkX--;
			}

			if (desiredPosC.offset.y > WORLD_CHUNK_SIZE / 2.0f)
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
				OffsetWorldPos(resultPos, V3(delta, 0.0f));

			for (i32 chunkY = minChunkY; chunkY <= maxChunkY; chunkY++)
			{
				for (i32 chunkX = minChunkX; chunkX <= maxChunkX; chunkX++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY);
					// TODO: Handle world edges
					if (chunk)
					{
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
									v2 relOldPos = WorldPosDiff(resultPos,
																testEntity->worldPos).xy;
									// TODO: Using aabb's and stuff
									v2 minCorner = -0.5f * V2(testEntity->size);
									v2 maxCorner = 0.5f * V2(testEntity->size);
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
			}
			
			resultPos = OffsetWorldPos(resultPos, V3(delta * tMin, 0.0f));
			
			if (hitEntityIndex)
			{
				delta = WorldPosDiff(targetPosition, resultPos).xy;
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
	DoEntityPhysicalMovement(World* world, HighEntity* highEntity,
							 v2 acceleration, Camera* camera,
							 MemoryArena* arena)
	{
		LowEntity* entity = GetLowEntity(world, highEntity->lowIndex);
		f32 speed = entity->accelerationAmount;
		acceleration *= speed;

		f32 friction = entity->friction;
		acceleration = acceleration - entity->velocity * friction;

		v2 movementDelta;
		movementDelta = 0.5f * acceleration *
			Square(GlobalGameDeltaTime) + 
			entity->velocity *
			GlobalGameDeltaTime;

		auto newPos = DoCollisionDetection(world, entity,  movementDelta);

		entity->velocity += acceleration * GlobalGameDeltaTime;
		Chunk* oldChunk = GetChunk(world, entity->worldPos.chunkX,
								   entity->worldPos.chunkY, nullptr);
		Chunk* newChunk = GetChunk(world, newPos.chunkX,
								   newPos.chunkY, nullptr);

		AB_ASSERT(oldChunk);
		AB_ASSERT(newChunk);
#if 0
		// TODO: Unified world position
		TileCoord oldTileCoord = ChunkRelOffsetToTileCoord(world,
														   entity->worldPos.offset);
		TileCoord newTileCoord = ChunkRelOffsetToTileCoord(world, newPos.offset);

		TerrainTileData oldTile = GetTerrainTile(world, oldChunk,
												 oldTileCoord.x,
												 oldTileCoord.y,
												 oldTileCoord.z);
		TerrainTileData newTile = GetTerrainTile(world, newChunk,
												 newTileCoord.x,
												 newTileCoord.y,
#endif											 newTileCoord.z);
												 // TODO: z
												 //f32 heightDiff = newTile->height - oldTile->height;
												 //newPos.offset.z += heightDiff;

												 ChangeEntityPos(world, entity, newPos, camera->targetWorldPos, arena);
												 highEntity->pos = GetCamRelPos(entity->worldPos,
																				camera->targetWorldPos);
												 }

			void
			MoveCamera(GameState* gameState, Camera* camera, World* world,
					   AssetManager* assetManager, MemoryArena* arena)
			{
			// TODO: Delete chunkMesh VBO when evicting chunk from high set
			// and make mesh again when chunk becomes high

			v2 entityFrameOffset = -MoveCameraTarget(&gameState->camera, world);
			UpdateCamera(camera, gameState->renderGroup, world);

			for (u32 i = 1; i < world->highEntityCount; i++)
			{
				world->highEntities[i].pos += V3(entityFrameOffset, 0.0f);
			}

			i32 highAreaChunkSpanX = 1;
			i32 highAreaChunkSpanY = 1;

			i32 minChunkX = SafeSubChunkCoord(camera->targetWorldPos.chunkX,
											  highAreaChunkSpanX);
			i32 minChunkY = SafeSubChunkCoord(camera->targetWorldPos.chunkY,
											  highAreaChunkSpanY);
			i32 maxChunkX = SafeAddChunkCoord(camera->targetWorldPos.chunkX,
											  highAreaChunkSpanX);
			i32 maxChunkY = SafeAddChunkCoord(camera->targetWorldPos.chunkY,
											  highAreaChunkSpanY); 
#if 0
			DEBUG_OVERLAY_TRACE(camera->targetWorldPos.chunkX);
			DEBUG_OVERLAY_TRACE(camera->targetWorldPos.chunkY);
			DEBUG_OVERLAY_TRACE(camera->targetWorldPos.offset.x);
			DEBUG_OVERLAY_TRACE(camera->targetWorldPos.offset.y);
#endif

			for (u32 i = 0; i < world->highChunkCount;)
			{
				Chunk* chunk = world->highChunks[i];
				if ((chunk->coordX < minChunkX) ||
					(chunk->coordX > maxChunkX) ||
					(chunk->coordY < minChunkY) ||
					(chunk->coordY > maxChunkY))
				{
					SetChunkToLow(world, chunk);
				}
				else
				{
					i++;
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
					SetChunkToHigh(world, chunk, camera->targetWorldPos);
				}
			}
			}
			}

			v2 minLine = WorldPosDiff({minChunkX, minChunkY, 0, 0},
									  camera->targetWorldPos).xy;

			v2 maxLine = WorldPosDiff({maxChunkX, maxChunkY, WORLD_CHUNK_SIZE, WORLD_CHUNK_SIZE},
									  camera->targetWorldPos).xy;

			DrawAlignedBoxOutline(gameState->renderGroup, assetManager,
								  V3(minLine.x, 3.0f, minLine.y),
								  V3(maxLine.x, 10.0f, maxLine.y),
								  V3(0.8, 0.0, 0.0), 2.0f);

			DEBUG_OVERLAY_TRACE(world->lowEntityCount);
			DEBUG_OVERLAY_TRACE(world->highEntityCount);
			}

		inline bool
			KeyJustPressed(KeyCode key)
		{
			bool result = GlobalInput.keys[key].pressedNow &&
				!GlobalInput.keys[key].wasPressed;
			return result;
		}
	
		void Render(MemoryArena* arena,
					MemoryArena* tempArena,
					GameState* gameState,
					AssetManager* assetManager,
					Renderer* renderer)
		{
			DEBUG_OVERLAY_TRACE(gameState->stringEnd);
			DEBUG_OVERLAY_TRACE(gameState->stringAt);
			if (KeyJustPressed(KEY_LEFT))
			{
				if (gameState->stringAt > 0)
				{
					gameState->stringAt--;
				}
			}

			if (KeyJustPressed(KEY_RIGHT))
			{
				if (gameState->stringAt < gameState->stringEnd)
				{
					gameState->stringAt++;
					AB_ASSERT(gameState->stringAt <= (STRING_SIZE));
				}
			}

			for (u32 i = 0; i < GlobalInput.textBufferCount; i++)
			{			
				char c = GlobalInput.textBuffer[i];
				if (c == '\b')
				{
					if (gameState->stringAt > 0 &&
						gameState->stringEnd > 0)
					{
						gameState->stringAt--;
						for (u32 i = gameState->stringAt;
							 i < gameState->stringEnd;
							 i++)
						{
							gameState->string[i] = gameState->string[i + 1];
						}
						gameState->string[gameState->stringEnd - 1] = '\0';
						gameState->stringEnd--;
					}
				}
				else
				{
					if (gameState->stringEnd < STRING_SIZE - 1 &&
						gameState->stringAt < STRING_SIZE - 1)
					{
						if (gameState->string[gameState->stringAt] != '\0')
						{
							if (gameState->stringEnd < STRING_SIZE - 1)
							{
								for (u32 i = gameState->stringEnd + 1;
									 i > gameState->stringAt;
									 i--)
								{
									gameState->string[i] = gameState->string[i - 1];
								}
							
							}
						}

						gameState->string[gameState->stringAt] = c;
						gameState->stringAt++;
						gameState->stringEnd++;
					}
				}
			}
		
			DEBUG_OVERLAY_STRING(gameState->string);
			DEBUG_OVERLAY_SLIDER(g_Platform->gameSpeed, 0.0f, 10.0f);
			World* world = gameState->world;
			Camera* camera = &gameState->camera;

			DEBUG_OVERLAY_TRACE(world->nonResidentEntityBlocksCount);
			DEBUG_OVERLAY_TRACE(world->freeEntityBlockCount);
			//DEBUG_OVERLAY_TRACE_VAR(gameState->selectedEntity);
			{

				MoveCamera(gameState, camera, world, assetManager, arena);
				//UpdateGizmos(gameState, arena);
				for (u32 index = 1; index < world->highEntityCount; index++)
				{
					Entity entity = GetEntityFromHighIndex(world, index);

					if (entity.low->type == ENTITY_TYPE_BODY)
					{
						DoEntityPhysicalMovement(world, entity.high , V2(0.0f),
												 camera, arena);
					}
				
					DrawEntity(gameState, assetManager, entity, false);

				}
			}

			//DEBUG_OVERLAY_TRACE(gameState->selectedTile.chunkX);
			//DEBUG_OVERLAY_TRACE(gameState->selectedTile.chunkY);
			//DEBUG_OVERLAY_TRACE(gameState->selectedTile.tileX);
			//DEBUG_OVERLAY_TRACE(gameState->selectedTile.tileX);

			if (GlobalInput.keys[KEY_T].pressedNow &&
				!GlobalInput.keys[KEY_T].wasPressed)
			{
				if (gameState->selectionMode == SELECTION_MODE_ENTITY)
				{
					gameState->selectionMode = SELECTION_MODE_TILEMAP;
				}
				else if (gameState->selectionMode == SELECTION_MODE_TILEMAP)
				{
					gameState->selectionMode = SELECTION_MODE_ENTITY;				
				}
				gameState->selectedEntity = nullptr;
				gameState->selectedTile = InvalidTileWorldPos();
			}

			if (gameState->selectionEnabled)
			{
				if (GlobalInput.mouseButtons[MBUTTON_LEFT].pressedNow &&
					!GlobalInput.mouseButtons[MBUTTON_LEFT].wasPressed)
				{
					if (gameState->selectionMode == SELECTION_MODE_ENTITY)
					{
						u32 hitIndex = RaycastFromCursor(camera, world);
						// TODO: IMPORTANT: Check if entity is high.
						// Only high entities can be selected

						gameState->selectedEntity = GetLowEntity(world, hitIndex);
						gameState->selectedTile = InvalidTileWorldPos();
					}
					else if (gameState->selectionMode == SELECTION_MODE_TILEMAP)
					{
						auto[hit, tMin, pos] = TilemapRaycast(world, camera,
															  camera->posWorld,
															  camera->mouseRayWorld);
						gameState->selectedTile = hit ? pos : InvalidTileWorldPos();
						gameState->selectedEntity = nullptr;
					}
					else
					{
						INVALID_CODE_PATH();
					}
				}

				if (gameState->selectedEntity)
				{
					if (GlobalInput.mouseButtons[MBUTTON_MIDDLE].pressedNow &&
						!GlobalInput.mouseButtons[MBUTTON_MIDDLE].wasPressed)
					{
						if (GetLowEntity(world, RaycastFromCursor(camera, world)) ==
							gameState->selectedEntity)
						{
							if (GlobalInput.keys[KEY_X].pressedNow)
							{
								gameState->dragAxis = MOUSE_DRAG_AXIS_X;
							}
							if (GlobalInput.keys[KEY_Y].pressedNow)
							{
								gameState->dragAxis = MOUSE_DRAG_AXIS_Y;
							}
							if (GlobalInput.keys[KEY_Z].pressedNow)
							{
							gameState->dragAxis = MOUSE_DRAG_AXIS_Z;
						}

						gameState->dragActive = true;
						v3 newPos = {};
						f32 t = (gameState->selectedEntity->worldPos.offset.z - camera->posWorld.z) / camera->mouseRayWorld.z;
						if (t >= 0.0f)
						{
							newPos.x = camera->posWorld.x + camera->mouseRayWorld.x * t;
							newPos.z = camera->posWorld.z + camera->mouseRayWorld.z * t;
							newPos.y = camera->posWorld.y + camera->mouseRayWorld.y * t;
						}
						v3 dragPos = {};
						switch (gameState->dragAxis)
						{
						case MOUSE_DRAG_AXIS_X: {dragPos.x = newPos.x;} break;
						case MOUSE_DRAG_AXIS_Y: {dragPos.y = newPos.y;} break;
						case MOUSE_DRAG_AXIS_Z:
						{
							newPos = camera->mouseRayWorld * gameState->yDragSpeed;
							dragPos.z = newPos.z;
						} break;

						INVALID_DEFAULT_CASE();
						}
						// TODO: Consistent basis
						gameState->prevDragPos = dragPos;
					}
				}
				else if (GlobalInput.mouseButtons[MBUTTON_MIDDLE].pressedNow &&
						 gameState->dragActive)
				{
					v3 newPos = {};
					f32 t = (gameState->selectedEntity->worldPos.offset.z - camera->posWorld.z) / camera->mouseRayWorld.z;
					if (t >= 0.0f)
					{
						newPos.x = camera->posWorld.x + camera->mouseRayWorld.x * t;
						newPos.z = camera->posWorld.z + camera->mouseRayWorld.z * t;
						newPos.y = camera->posWorld.y + camera->mouseRayWorld.y * t;
						//f32 offset.y = from.y + dir.y * t;
					}
					v3 dragPos = {};
					switch (gameState->dragAxis)
					{
					case MOUSE_DRAG_AXIS_X: {dragPos.x = newPos.x;} break;
					case MOUSE_DRAG_AXIS_Y: {dragPos.y = newPos.y;} break;
					case MOUSE_DRAG_AXIS_Z:
					{
						newPos = camera->mouseRayWorld * gameState->yDragSpeed;
						dragPos.z = newPos.z;
					} break;

					INVALID_DEFAULT_CASE();
					}
					v3 offset = dragPos - gameState->prevDragPos;
					OffsetEntityPos(world, gameState->selectedEntity,
									offset, camera->targetWorldPos, arena);
					gameState->prevDragPos = dragPos;
				} else if (!GlobalInput.mouseButtons[MBUTTON_MIDDLE].pressedNow &&
						   GlobalInput.mouseButtons[MBUTTON_MIDDLE].wasPressed)
				{
					gameState->dragActive = false;
					gameState->dragAxis = MOUSE_DRAG_AXIS_NULL;
				}
			}
		}
		{
			auto renderer = g_StaticStorage->debugRenderer;
			Rectangle windowBox = RectLeftCornerDim(V2(22.0f, 298.0f),
													V2(205.0f, 254.0f));
			v2 mousePos = V2(GlobalInput.mouseX, GlobalInput.mouseY);
			mousePos.x *= PlatformGlobals.windowWidth;
			mousePos.y *= PlatformGlobals.windowHeight;
			DEBUG_OVERLAY_TRACE(PlatformGlobals.windowWidth);
			DEBUG_OVERLAY_TRACE(PlatformGlobals.windowHeight);			
			Renderer2DFillRectangleColor(renderer, mousePos, 1, 0.0f,
										 0.0f, V2(5.0f, 5.0f), 0xff171a39);

			if (Contains(windowBox, mousePos))
			{
				gameState->selectionEnabled = false;
			}
			else
			{
				gameState->selectionEnabled = true;				
			}
			
			Renderer2DFillRectangleColor(renderer, V2(25.0f, 300.0f), 1, 0.0f,
										 0.0f, V2(200.0f, 250.0f), 0xff171a39);
			Renderer2DFillRectangleColor(renderer, V2(22.0f, 298.0f), 0, 0.0f,
										 0.0f, V2(205.0f, 254.0f), 0xff111111);
			Renderer2DFillRectangleColor(renderer, V2(25.0f, 510.0f), 2, 0.0f,
										 0.0f, V2(200.0f, 40.0f), 0xff4dbf00);
			Renderer2DDebugDrawString(renderer, V2(38.0f, 540.0f), 24,
									  0xffffffff, "Tile properties");


		}

		DEBUG_OVERLAY_STRING("");
		DEBUG_OVERLAY_STRING(gameState->selectionMode == SELECTION_MODE_ENTITY ? "ENTITIES" : "TILEMAP");
		if (gameState->selectionMode == SELECTION_MODE_TILEMAP)
		{
			if (IsValid(gameState->selectedTile))
			{
				DEBUG_OVERLAY_TRACE(gameState->selectedTile.tileX);
				DEBUG_OVERLAY_TRACE(gameState->selectedTile.tileY);
				Chunk* chunk = GetChunk(world, gameState->selectedTile.chunkX,
										gameState->selectedTile.chunkY);
				TerrainTileData tile = GetTerrainTile(chunk, 
													  gameState->selectedTile.tileX,
													  gameState->selectedTile.tileY,
													  gameState->selectedTile.tileZ);
				DEBUG_OVERLAY_PUSH_SLIDER("Tile type", (u32*)(&tile.type), 0, 3);

				SetTerrainTile(chunk, gameState->selectedTile.tileX,
							   gameState->selectedTile.tileY,
							   gameState->selectedTile.tileZ, &tile);
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

			DoEntityPhysicalMovement(world, entity.high,
									 acceleration, camera, arena);
			//DEBUG_OVERLAY_TRACE(entity.low->worldPos.chunkX);
			//DEBUG_OVERLAY_TRACE(entity.low->worldPos.chunkY);
			//DEBUG_OVERLAY_TRACE(entity.low->worldPos.offset.x);
			//DEBUG_OVERLAY_TRACE(entity.low->worldPos.offset.y);
			TileCoord tileCoord = ChunkRelOffsetToTileCoord(entity.low->worldPos.offset);
			//DEBUG_OVERLAY_TRACE(tileCoord.x);
			//DEBUG_OVERLAY_TRACE(tileCoord.y);
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
			DoEntityPhysicalMovement(world, entity1.high,
									 acceleration, camera, arena);
			//DEBUG_OVERLAY_TRACE(entity1.low->worldPos.chunkX);
			//DEBUG_OVERLAY_TRACE(entity1.low->worldPos.chunkY);
			//DEBUG_OVERLAY_TRACE(entity1.low->worldPos.offset.x);
			//DEBUG_OVERLAY_TRACE(entity1.low->worldPos.offset.y);
		}
		
#if 0
		DrawWorldInstanced(world, gameState->renderGroup, assetManager,
						   gameState->camera.targetWorldPos,
						   &gameState->selectedTile);
#else
		MesherUpdateChunks(world->chunkMesher, world, tempArena);
		MesherDrawChunks(world->chunkMesher, world, camera,
						 gameState->renderGroup, assetManager);
		DrawBlockHighlight(gameState->renderGroup, assetManager, world, camera,
						   gameState->selectedTile);

#endif

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
