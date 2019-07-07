#include "Sandbox.h"
// TODO: This is temporary for rand()
#include <stdlib.h>


namespace AB
{

	void DrawBlockHighlight(RenderGroup* renderGroup,
							AssetManager* assetManager,
							World* world,
							Camera* camera, ChunkPosition* pos)
	{
		if (IsValid(pos))
		{
			v3 offset = V3(0.0f);
			Chunk* chunk = GetChunk(world, pos->chunk);
			
			offset = GetRelativePos(&camera->targetWorldPos, &GetWorldPos(pos));
#if 0
			offset.x = (chunk->coordX * WORLD_CHUNK_DIM_TILES - camera->targetWorldPos.tile.x) * WORLD_TILE_SIZE;
			offset.x += camera->targetWorldPos.offset.x;
			offset.y = (chunk->coordY * WORLD_CHUNK_DIM_TILES - camera->targetWorldPos.tile.y) * WORLD_TILE_SIZE;
			offset.y += camera->targetWorldPos.offset.y;
			//offset.y += pos.tileY * WORLD_TILE_SIZE;
			offset.z = (chunk->coordZ * WORLD_CHUNK_DIM_TILES - camera->targetWorldPos.tile.z) * WORLD_TILE_SIZE;
			offset.z -= camera->targetWorldPos.offset.z;
			//offset.z += pos.tileZ * WORLD_TILE_SIZE;
#endif
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
		
		gameState->camera.targetWorldPos.tile.x = 0;
		gameState->camera.targetWorldPos.tile.x = 0;
		gameState->camera.targetWorldPos.offset.x = 0.5f;
		gameState->camera.targetWorldPos.offset.y = 0.5f;

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
				Chunk* chunk0 = GetChunk(world, V3I(x, y, 0), arena);
				Chunk* chunkM1 = GetChunk(world, V3I(x, y, -1), arena);
				Chunk* chunkP1 = GetChunk(world, V3I(x, y, 1), arena);
				
				for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
				{
					for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
					{
						for (u32 tileZ = 0; tileZ < WORLD_CHUNK_DIM_TILES; tileZ++)
						{
							u32 bias = 0;
							if (tileZ == 15)
							{
								bias = rand() % 2;									
							}

							TerrainTileData tileM1 = GetTerrainTile(chunkM1, V3U(tileX, tileY, tileZ));
							tileM1.type = TERRAIN_TYPE_CLIFF;
							SetTerrainTile(chunkM1, V3U(tileX, tileY, tileZ), &tileM1);

							if (tileZ  < 16)
							{
								TerrainTileData tile0 = GetTerrainTile(chunk0, V3U(tileX, tileY, tileZ + bias));

								if ((x == -4 && tileX == 0) ||
									(x == 3 &&
									 tileX == WORLD_CHUNK_DIM_TILES - 1) ||
									(y == -4 && tileY == 0) ||
									(y == 3 &&
									 tileY == WORLD_CHUNK_DIM_TILES - 1))
								{
									tile0.type = TERRAIN_TYPE_CLIFF;
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
										tile0.type = TERRAIN_TYPE_GRASS;									
									}
									else
									{
										tile0.type = TERRAIN_TYPE_CLIFF;									
									}
							
								}
								SetTerrainTile(chunk0, V3U(tileX, tileY, tileZ + bias), &tile0);
							}
						}
					}
				}
			}
		}

		{
			Chunk* chunk = GetChunk(world, V3I(-3, -4, 0), arena);
			for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
			{
				for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
				{
					TerrainTileData tile = GetTerrainTile(chunk, V3U(tileX, tileY, 0));
					tile.type = TERRAIN_TYPE_WATER;
					SetTerrainTile(chunk, V3U(tileX, tileY, 0), &tile);
				}
			}

		}

		Chunk* firstChunk = GetChunk(gameState->world, V3I(0, 0, 0));
		AB_ASSERT(firstChunk);
#if 1
		MesherUpdateChunks(world->chunkMesher, world, tempArena);
#endif

		gameState->entity = AddStoredEntity(world, firstChunk,
										 ENTITY_TYPE_BODY, arena);
		StoredEntity* e = GetStoredEntity(world, gameState->entity);
		e->worldPos.tile.x = 10;
		e->worldPos.tile.y = 10;
		e->worldPos.tile.z = 20;
		e->worldPos.offset = V3(0.0f);
		e->storage.accelerationAmount = 20.0f;
		e->storage.size = 0.5f;
#if 0
		e->storage.meshes[0] = assetManager->meshes + gameState->treeFoliageHandle;
		e->storage.meshes[1] = assetManager->meshes + gameState->treeTrunkHandle;
		e->storage.meshCount = 2;
#endif
		e->storage.meshes[0] = GetMesh(assetManager, 0);
		e->storage.meshCount = 1;
		
		e->storage.color = V3(1.0f, 0.0f, 0.0f);
		e->storage.friction = 3.0f;
		//e->stored.velocity = V3(0.0f, 0.1f, 0.2f);
		
		gameState->entity1 = AddStoredEntity(world, firstChunk,
										  ENTITY_TYPE_BODY, arena);
		StoredEntity* e1 = GetStoredEntity(world, gameState->entity1);
		e1->worldPos.offset = V3(0.6f, 0.5f, 0.5f);
		e1->storage.accelerationAmount = 30.0f;
		e1->storage.size = 2.0f;
		e1->storage.color = V3(0.0f, 1.0f, 0.0f);
		e1->storage.friction = 3.0f;
		e1->storage.type = ENTITY_TYPE_BODY;
		e1->storage.meshes[0] = assetManager->meshes + gameState->treeFoliageHandle;
		e1->storage.meshes[1] = assetManager->meshes + gameState->treeTrunkHandle;
		e1->storage.meshCount = 2;


#if 0
		for (u32 i = 0; i < MOVING_ENTITIES_COUNT; i++)
		{
			u32 id = AddStoredEntity(world, firstChunk,
								  ENTITY_TYPE_BODY, arena);
			gameState->movingEntities[i] = id; 
			StoredEntity* e = GetStoredEntity(world, id);
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
	DrawEntity(GameState* gameState, AssetManager* assetManager, SimRegion* region,
			   Entity* entity, bool drawBBox = false)
	{
		// TODO: Setting camera on sim region concept
		WorldPosition wPos = GetWorldPos(region, entity->pos);
		v3 camRelPos = GetRelativePos(&gameState->camera.targetWorldPos, &wPos);
		v3 pos = V3(0.0f);
		pos.x = camRelPos.x;
		pos.z = camRelPos.y;

		v3 scale = V3(0.0f);
		scale.x = entity->size;
		scale.z = entity->size;
		scale.y = entity->size;
		// TODO: Mesh aligment, tilemap footprints n' stuff
		pos.y = camRelPos.z;

		//pos.x -= scale.x * 0.5f;
		//pos.z -= scale.z * 0.5f;
		v3 color = entity->color;
		bool selected = false;
		if (entity->id == gameState->selectedEntityID)
		{
			selected = true;
		}
		for (u32 i = 0; i < entity->meshCount; i++)
		{
			Mesh* mesh = entity->meshes[i];
			DrawDebugMesh(gameState->renderGroup, assetManager, pos, scale,
						  mesh, selected);
			if (drawBBox || selected)
			{
				v3 worldPos = FlipYZ(camRelPos);
				v3 min = mesh->aabb.min * entity->size + worldPos;
				v3 max = mesh->aabb.max * entity->size + worldPos;

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

	struct EntityMovement
	{
		v3 dest;
		v3 newVelocity;
	};

	EntityMovement
	ComputeEntityMovement(SimRegion* region, Entity* entity, v3 delta)
	{
		EntityMovement result;
		result.dest = entity->pos;
		result.newVelocity = entity->velocity;
		// TODO: bounding boxes
		v3 colliderSize = V3(entity->size);
		v3 iterationOffset = {};

		const f32 tEps = 0.01f;

		for (u32 pass = 0; pass < 4; pass++)
		{
			v3 wallNormal = V3(0.0f);
			f32 tMin = 1.0f;
			bool hit = false;
			v3 targetPosition = entity->pos + delta;
			
			// TODO: Handle case when entity moved out of sim region boundaries
			
			for (u32 simIndex = 0;
				 simIndex < region->entityCount;
				 simIndex++)
			{
				Entity* testEntity = region->entities + simIndex;
							
				if (entity != testEntity)
				{
					// TODO: Using aabb's and stuff
					v3 minCorner = -0.5f * V3(testEntity->size) + testEntity->pos;
					v3 maxCorner = 0.5f * V3(testEntity->size) + testEntity->pos;
					//NOTE: Minkowski sum
					minCorner += colliderSize * -0.5f;
					maxCorner += colliderSize * 0.5f;
					auto raycast = RayAABBIntersection(entity->pos, delta,
													   minCorner, maxCorner);
					if (raycast.hit)
					{
						raycast.tMin = MAXIMUM(0.0f, raycast.tMin - tEps);

						if (raycast.tMin < tMin)
						{
							tMin = raycast.tMin;
							wallNormal = raycast.normal;
							hit = true;												
						}
					}
				}
					
			}
			
			result.dest = result.dest +  delta * tMin;

			if (hit)
			{
				delta = targetPosition - result.dest;
				result.newVelocity -= 2.0f * Dot(result.newVelocity, wallNormal) * wallNormal;
				// TODO: Should it be here
				//delta *= 1.0f - tMin;
				delta -= Dot(delta, wallNormal) * wallNormal;				
			}
			else
			{
				break;
			}
		}
		return result;
	}

	void
	MoveEntity(SimRegion* region, Entity* entity, v3 acceleration)
	{
		f32 speed = entity->accelerationAmount;
		acceleration *= speed;

		f32 friction = entity->friction;
		acceleration = acceleration - entity->velocity * friction;

		v3 movementDelta = 0.5f * acceleration *
			Square(GlobalGameDeltaTime) + 
			entity->velocity *
			GlobalGameDeltaTime;
		EntityMovement movement =
			ComputeEntityMovement(region, entity, movementDelta);
		
		entity->pos = movement.dest;
		entity->velocity = movement.newVelocity;
		entity->velocity += acceleration * GlobalGameDeltaTime;
	}


	inline bool
	KeyJustPressed(KeyCode key)
	{
		bool result = GlobalInput.keys[key].pressedNow &&
			!GlobalInput.keys[key].wasPressed;
		return result;
	}

	void DoStringEditingStuff(GameState* gameState)
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
	}
	
	void MoveCamera(GameState* gameState, Camera* camera, World* world,
					AssetManager* assetManager)
	{
		ChunkMesher* mesher = world->chunkMesher;
		MoveCameraTarget(camera, world);
		UpdateCamera(camera, gameState->renderGroup, world);

		v3i areaSpan = V3I(1);

		ChunkRegion region =
			ChunkRegionFromOriginAndSpan(GetChunkPos(&camera->targetWorldPos).chunk, areaSpan);

		for (u32 i = 0; i < mesher->chunkCount;)
		{
			Chunk* chunk = mesher->chunks[i];
			if ((chunk->coordX < region.minBound.x) ||
				(chunk->coordX > region.maxBound.x) ||
				(chunk->coordY < region.minBound.y) ||
				(chunk->coordY > region.maxBound.y) ||
				(chunk->coordZ < region.minBound.z) ||
				(chunk->coordZ > region.maxBound.z))
			{
				AB_ASSERT(chunk->visible);
				chunk->visible = false;
				MesherRemoveChunk(world->chunkMesher, chunk);
			}
			else
			{
				i++;
			}
		}

		for (i32 chunkZ = region.minBound.z; chunkZ <= region.maxBound.z; chunkZ++)
		{
			for (i32 chunkY = region.minBound.y; chunkY <= region.maxBound.y; chunkY++)
			{
				for (i32 chunkX = region.minBound.x; chunkX <= region.maxBound.x; chunkX++)
				{
					Chunk* chunk = GetChunk(world, V3I(chunkX, chunkY, chunkZ));
					if (chunk)
					{
						if (!chunk->visible)
						{
							MesherAddChunk(world->chunkMesher, chunk);
							chunk->visible = true;
						}
					}
				}
			}
		}
		
		region.minBound *= WORLD_CHUNK_DIM_TILES;
		region.maxBound *= WORLD_CHUNK_DIM_TILES;

		WorldPosition mn = {region.minBound.x, region.minBound.y, region.minBound.z,  0, 0};
		WorldPosition mx = {region.maxBound.x + (i32)WORLD_CHUNK_DIM_TILES,
							region.maxBound.y + (i32)WORLD_CHUNK_DIM_TILES,
							region.maxBound.z + (i32)WORLD_CHUNK_DIM_TILES, 0, 0};
		v3 minLine = WorldPosDiff(&mn, &camera->targetWorldPos);

		v3 maxLine = WorldPosDiff(&mx, &camera->targetWorldPos);

		DrawAlignedBoxOutline(gameState->renderGroup, assetManager,
							  FlipYZ(minLine),
							  FlipYZ(maxLine),
							  V3(0.8, 0.0, 0.0), 2.0f);

		DEBUG_OVERLAY_TRACE(world->lowEntityCount);
	}

	
	void Render(MemoryArena* arena,
				MemoryArena* tempArena,
				GameState* gameState,
				AssetManager* assetManager,
				Renderer* renderer)
	{
		DoStringEditingStuff(gameState);
		World* world = gameState->world;
		Camera* camera = &gameState->camera;
		RenderGroup* renderGroup = gameState->renderGroup;

		MoveCamera(gameState, camera, world, assetManager);

		BeginTemporaryMemory(tempArena);
		SimRegion* region =
			BeginSim(tempArena, world, camera->targetWorldPos, V3I(1, 1, 1));

		for (u32 simEntityIndex = 0;
			 simEntityIndex < region->entityCount;
			 simEntityIndex++)
		{
			Entity* entity = region->entities + simEntityIndex;
			//MoveEntity(region, entity, V3(0.0f));
			DrawEntity(gameState, assetManager, region, entity);
		}

		Entity* entity1 = GetSimEntity(region, gameState->entity);
		Entity* entity2 = GetSimEntity(region, gameState->entity1);
		
		if (entity1)
		{ // Enitity 1 movement code
			StoredEntity* st = GetStoredEntity(world, gameState->entity);
			DEBUG_OVERLAY_TRACE(st->worldPos.tile.x);
			DEBUG_OVERLAY_TRACE(st->worldPos.tile.y);
			DEBUG_OVERLAY_TRACE(st->worldPos.tile.z);
			DEBUG_OVERLAY_TRACE(st->worldPos.offset);
			DEBUG_OVERLAY_TRACE(GetChunkPos(&st->worldPos).tile.x);
			DEBUG_OVERLAY_TRACE(GetChunkPos(&st->worldPos).tile.y);
			DEBUG_OVERLAY_TRACE(GetChunkPos(&st->worldPos).tile.z);
			DEBUG_OVERLAY_TRACE(GetChunkPos(&st->worldPos).chunk.x);
			DEBUG_OVERLAY_TRACE(GetChunkPos(&st->worldPos).chunk.y);
			DEBUG_OVERLAY_TRACE(GetChunkPos(&st->worldPos).chunk.z);
			
			v3 _frontDir = V3(camera->front.x, 0.0f, camera->front.z);
			v3 _rightDir = Cross(V3(0.0f, 1.0f, 0.0f), _frontDir);
			_rightDir = Normalize(_rightDir);

			v3 frontDir = V3(_frontDir.x, _frontDir.z, 0.0f);
			v3 rightDir = V3(_rightDir.x, _rightDir.z, 0.0f);
			v3 upDir  = V3(0.0f, 0.0f, 1.0f);

			v3 acceleration = {};
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
			if (GlobalInput.keys[KEY_SPACE].pressedNow)
			{
				acceleration += upDir;
			}
			if (GlobalInput.keys[KEY_SHIFT].pressedNow)
			{
				acceleration -= upDir;
			}
		 
			acceleration = Normalize(acceleration);

			MoveEntity(region, entity1, acceleration);
			DEBUG_OVERLAY_TRACE(entity1->velocity);

		}

		if (entity2)
		{ // Enitity 2 movement code
			v3 _frontDir = V3(camera->front.x, 0.0f, camera->front.z);
			v3 _rightDir = Cross(V3(0.0f, 1.0f, 0.0f), _frontDir);
			_rightDir = Normalize(_rightDir);

			v3 frontDir = V3(_frontDir.x, _frontDir.z, 0.0f);
			v3 rightDir = V3(_rightDir.x, _rightDir.z, 0.0f);
			v3 upDir = V3(0.0f, 0.0f, 1.0f);

			v3 acceleration = {};
			if (GlobalInput.keys[KEY_NUM8].pressedNow)
			{
				acceleration += frontDir;
			}
			if (GlobalInput.keys[KEY_NUM5].pressedNow)
			{
				acceleration -= frontDir;
			}
			if (GlobalInput.keys[KEY_NUM4].pressedNow)
			{
				acceleration -= rightDir;
			}
			if (GlobalInput.keys[KEY_NUM6].pressedNow)
			{
				acceleration += rightDir;
			}
			if (GlobalInput.keys[KEY_NUM_ADD].pressedNow)
			{
				acceleration += upDir;
			}
			if (GlobalInput.keys[KEY_NUM_SUBTRACT].pressedNow)
			{
				acceleration -= upDir;
			}
		
			acceleration = Normalize(acceleration);
			MoveEntity(region, entity2, acceleration);
		}

		MesherUpdateChunks(world->chunkMesher, world, tempArena);
		MesherDrawChunks(world->chunkMesher, world, camera,
						 gameState->renderGroup, assetManager);
		DrawBlockHighlight(gameState->renderGroup, assetManager, world, camera,
						   &gameState->selectedTile);

		// TODO: Cleanup. Why axises are flipped?
		// Fix assert when moving selected entity out of sim region
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
			gameState->selectedEntityID = 0;
			gameState->selectedTile = InvalidChunkPos();
		}

		if (gameState->selectionEnabled)
		{
			if (GlobalInput.mouseButtons[MBUTTON_LEFT].pressedNow &&
				!GlobalInput.mouseButtons[MBUTTON_LEFT].wasPressed)
			{
				if (gameState->selectionMode == SELECTION_MODE_ENTITY)
				{
					u32 hitIndex = RaycastFromCursor(region, camera);
					gameState->selectedEntityID = hitIndex;
					gameState->selectedTile = InvalidChunkPos();
				}
				else if (gameState->selectionMode == SELECTION_MODE_TILEMAP)
				{
					auto[hit, tMin, pos, normal] =
						TilemapRaycast(region, camera->posWorld, camera->mouseRayWorld);
					gameState->selectedTile = hit ? pos : InvalidChunkPos();
					gameState->selectedEntityID = 0;
				}
				else
				{
					INVALID_CODE_PATH();
				}
			}
			
			if (gameState->selectedEntityID)
			{
				if (GlobalInput.mouseButtons[MBUTTON_MIDDLE].pressedNow &&
					!GlobalInput.mouseButtons[MBUTTON_MIDDLE].wasPressed)
				{
					if (RaycastFromCursor(region, camera) ==
						gameState->selectedEntityID)
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
						Entity* entity =
							GetSimEntity(region, gameState->selectedEntityID);
						v3 newPos = {};
						f32 t = (entity->pos.z - camera->pos.z) / camera->mouseRay.z;
						if (t >= 0.0f)
						{
							newPos.x = camera->pos.x + camera->mouseRay.x * t;
							newPos.z = camera->pos.z + camera->mouseRay.z * t;
							newPos.y = camera->pos.y + camera->mouseRay.y * t;
						}
						v3 dragPos = {};
						switch (gameState->dragAxis)
						{
						case MOUSE_DRAG_AXIS_X: {dragPos.x = newPos.x;} break;
						case MOUSE_DRAG_AXIS_Y: {dragPos.y = newPos.y;} break;
						case MOUSE_DRAG_AXIS_Z:
						{
							newPos = camera->mouseRay * gameState->yDragSpeed;
							dragPos.z = newPos.z;
						} break;

						INVALID_DEFAULT_CASE();
						}
						gameState->prevDragPos = dragPos;
					}
				}
				else if (GlobalInput.mouseButtons[MBUTTON_MIDDLE].pressedNow &&
						 gameState->dragActive)
				{
					Entity* entity =
						GetSimEntity(region, gameState->selectedEntityID);
					v3 newPos = {};
					f32 t = (entity->pos.z - camera->pos.z) / camera->mouseRay.z;
					if (t >= 0.0f)
					{
						newPos.x = camera->pos.x + camera->mouseRay.x * t;
						newPos.z = camera->pos.z + camera->mouseRay.z * t;
						newPos.y = camera->pos.y + camera->mouseRay.y * t;
						//f32 offset.y = from.y + dir.y * t;
					}
					v3 dragPos = {};
					switch (gameState->dragAxis)
					{
					case MOUSE_DRAG_AXIS_X: {dragPos.x = newPos.x;} break;
					case MOUSE_DRAG_AXIS_Y: {dragPos.y = newPos.y;} break;
					case MOUSE_DRAG_AXIS_Z:
					{
						newPos = camera->mouseRay * gameState->yDragSpeed;
						dragPos.z = newPos.z;
					} break;

					INVALID_DEFAULT_CASE();
					}
					v3 offset = dragPos - gameState->prevDragPos;
					entity->pos += offset;
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
			if (IsValid(&gameState->selectedTile))
			{
				DEBUG_OVERLAY_TRACE(GetWorldPos(&gameState->selectedTile).tile.x);
				DEBUG_OVERLAY_TRACE(GetWorldPos(&gameState->selectedTile).tile.y);
				DEBUG_OVERLAY_TRACE(GetWorldPos(&gameState->selectedTile).tile.z);
				Chunk* chunk = GetChunk(world, gameState->selectedTile.chunk);
				TerrainTileData tile = GetTerrainTile(chunk, gameState->selectedTile.tile);
				DEBUG_OVERLAY_PUSH_SLIDER("Tile type", (u32*)(&tile.type), 0, 3);

				SetTerrainTile(chunk, gameState->selectedTile.tile, &tile);
			}
		}

		EndSim(region, world, arena);
		
		EndTemporaryMemory(tempArena);
		
		DEBUG_OVERLAY_SLIDER(g_Platform->gameSpeed, 0.0f, 10.0f);
		DEBUG_OVERLAY_TRACE(world->nonResidentEntityBlocksCount);
		DEBUG_OVERLAY_TRACE(world->freeEntityBlockCount);

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

#if 0
void


WorldPosition
DoCollisionDetection(World* world, Camera* camera, StoredEntity* entity, v3 delta)
{
	WorldPosition resultPos = entity->worldPos;
	// TODO: bounding boxes
	v3 colliderSize = V3(entity->size);
	v3 iterationOffset = {};

	const f32 tEps = 0.01f;

	for (u32 pass = 0; pass < 1; pass++)
	{
		v3 wallNormal = V3(0.0f);
		f32 tMin = 1.0f;
		bool hit = false;
		WorldPosition targetPosition = OffsetWorldPos(resultPos, delta);
		// NOTE: Too slow for now
#if 0
		// NOTE: Raycating against tilemap here
		v3 camRelPos = GetCamRelPos(resultPos, camera->targetWorldPos);
		auto tilemapRaycast = TilemapRaycast(world, camera, camRelPos, delta);
		if (tilemapRaycast.hit)
		{
			tilemapRaycast.tMin = MAXIMUM(0.0f, tilemapRaycast.tMin - tEps);
			if (tilemapRaycast.tMin < tMin)
			{
				tMin = tilemapRaycast.tMin;
				wallNormal = tilemapRaycast.normal;
				hit = true;
			}
		}
#endif
		// TODO: Using AABB's to calculate chunks for checking
		// TODO: This is totally crappy code for collision
		// detection on chunk bounds
		// Needs to be rapaced with robust code.
		WorldPosition beginPos = resultPos;
		WorldPosition desiredPosP = OffsetWorldPos(beginPos, delta + colliderSize);

		WorldPosition desiredPosN = OffsetWorldPos(beginPos, delta - colliderSize);

		WorldPosition desiredPosC =	OffsetWorldPos(beginPos, delta);

		i32 minChunkX = MINIMUM(beginPos.chunkX, MINIMUM(desiredPosP.chunkX,
														 desiredPosN.chunkX));
		i32 maxChunkX = MAXIMUM(beginPos.chunkX, MAXIMUM(desiredPosP.chunkX,
														 desiredPosN.chunkX));
		i32 minChunkY = MINIMUM(beginPos.chunkY, MINIMUM(desiredPosP.chunkY,
														 desiredPosN.chunkY));
		i32 maxChunkY = MAXIMUM(beginPos.chunkY, MAXIMUM(desiredPosP.chunkY,
														 desiredPosN.chunkY));
		i32 minChunkZ = MINIMUM(beginPos.chunkZ, MINIMUM(desiredPosP.chunkZ,
														 desiredPosN.chunkZ));
		i32 maxChunkZ = MAXIMUM(beginPos.chunkZ, MAXIMUM(desiredPosP.chunkZ,
														 desiredPosN.chunkZ));
	  
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
			
		if (desiredPosC.offset.z > WORLD_CHUNK_SIZE / 2.0f)
		{
			maxChunkZ++;
		}
		else
		{
			minChunkZ--;
		}
			
		for (i32 chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++)
		{
			for (i32 chunkY = minChunkY; chunkY <= maxChunkY; chunkY++)
			{
				for (i32 chunkX = minChunkX; chunkX <= maxChunkX; chunkX++)
				{
					Chunk* chunk = GetChunk(world, chunkX, chunkY, chunkZ);
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
								StoredEntity* testEntity =
									GetStoredEntity(world, testEntityIndex);
							
								if (entity != testEntity)
								{
									v3 relOldPos = WorldPosDiff(resultPos,
																testEntity->worldPos);
									// TODO: Using aabb's and stuff
									v3 minCorner = -0.5f * V3(testEntity->size);
									v3 maxCorner = 0.5f * V3(testEntity->size);
									//NOTE: Minkowski sum
									minCorner += colliderSize * -0.5f;
									maxCorner += colliderSize * 0.5f;
									auto result = RayAABBIntersection(relOldPos, delta,
																	  minCorner, maxCorner);
									if (result.hit)
									{
										result.tMin = MAXIMUM(0.0f, result.tMin - tEps);

										if (result.tMin < tMin)
										{
											tMin = result.tMin;
											wallNormal = result.normal;
											hit = true;												
										}
									}
								}
							}
							block = block->nextBlock;
						} while (block);
					
					}
				}
			}
		}
			
		resultPos = OffsetWorldPos(resultPos, delta * tMin);
		wallNormal = -wallNormal;
			
		if (hit)
		{
			delta = WorldPosDiff(targetPosition, resultPos);
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
						 v3 acceleration, Camera* camera,
						 MemoryArena* arena)
{
	StoredEntity* entity = GetStoredEntity(world, highEntity->lowIndex);
	f32 speed = entity->accelerationAmount;
	acceleration *= speed;
	//acceleration.z += -9.8f;

	f32 friction = entity->friction;
	acceleration = acceleration - entity->velocity * friction;

	v3 movementDelta;
	movementDelta = 0.5f * acceleration *
		Square(GlobalGameDeltaTime) + 
		entity->velocity *
		GlobalGameDeltaTime;

	auto newPos = DoCollisionDetection(world, camera, entity,  movementDelta);

	entity->velocity += acceleration * GlobalGameDeltaTime;
	Chunk* oldChunk = GetChunk(world, entity->worldPos.chunkX,
							   entity->worldPos.chunkY, 0, nullptr);
	Chunk* newChunk = GetChunk(world, newPos.chunkX,
							   newPos.chunkY, 0, nullptr);

	AB_ASSERT(oldChunk);
	AB_ASSERT(newChunk);
#if 0
	// TODO: Unified world position
	TileCoord oldTileCoord = ChunkRelOffsetToTileCoord(world, entity->worldPos.offset);
	TileCoord newTileCoord = ChunkRelOffsetToTileCoord(world, newPos.offset);

	TerrainTileData oldTile = GetTerrainTile(world, oldChunk,
											 oldTileCoord.x,
											 oldTileCoord.y,
											 oldTileCoord.z);
	TerrainTileData newTile = GetTerrainTile(world, newChunk,
											 newTileCoord.x,
											 newTileCoord.y,
											 newTileCoord.z);
#endif
	// TODO: z
	//f32 heightDiff = newTile->height - oldTile->height;
	//newPos.offset.z += heightDiff;

	ChangeEntityPos(world, entity, newPos, camera->targetWorldPos, arena);
	highEntity->pos = GetCamRelPos(entity->worldPos,
								   camera->targetWorldPos);
}


#endif
