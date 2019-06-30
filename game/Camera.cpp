#include "Camera.h"

namespace AB
{

	u32 RaycastFromCursor(Camera* camera, World* world)
	{
		u32 hitEntityIndex = 0;
		v3 dir = camera->mouseRayWorld;
		v3 from = camera->posWorld;
		hitEntityIndex = Raycast(world, camera, from, dir);			
		return hitEntityIndex;
	}

	// TODO: Get rid of this
	b32 TestWall(f32 wallX, f32 relPlayerX, f32 relPlayerY,
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

	v2 MoveCameraTarget(Camera* camera, World* world)
	{
		v2 camFrameOffset = {};
		
		v3 _frontDir = V3(camera->front.x, 0.0f, camera->front.z);
		v3 _rightDir = Cross(V3(0.0f, 1.0f, 0.0f), _frontDir);
		// TODO: Is that normalization is neccessary?
		_rightDir = Normalize(_rightDir);

		v2 frontDir = V2(_frontDir.x, _frontDir.z);
		v2 rightDir = V2(_rightDir.x, _rightDir.z);

		v2 acceleration = {};
		if (!camera->debugMode)
		{
			if (GlobalInput.keys[KEY_W].pressedNow)
			{
				acceleration += frontDir;
			}
			if (GlobalInput.keys[KEY_S].pressedNow)
			{
				acceleration -= frontDir;
			}
			if (GlobalInput.keys[KEY_A].pressedNow)
			{
				acceleration -= rightDir;
			}
			if (GlobalInput.keys[KEY_D].pressedNow)
			{
				acceleration += rightDir;
			}
		}

		acceleration = Normalize(acceleration);
		f32 speed = 500.0f;
		acceleration *= speed;

		f32 friction = 8.0f;
		acceleration = acceleration - camera->targetWorldVelocity * friction;

		v2 movementDelta;
		movementDelta = 0.5f * acceleration *
			Square(GlobalAbsDeltaTime) + 
			camera->targetWorldVelocity *
			GlobalAbsDeltaTime;

		WorldPosition newPos = {};
		v2 newVelocity = {};

		newPos = OffsetWorldPos(camera->targetWorldPos,
								V3(movementDelta, 0.0f));
		newVelocity = camera->targetWorldVelocity;

		newVelocity = newVelocity +
			acceleration * GlobalAbsDeltaTime;

		camFrameOffset = WorldPosDiff(newPos, camera->targetWorldPos).xy;
		camera->targetWorldPos = newPos;
		camera->targetWorldVelocity = newVelocity;

		return camFrameOffset;
	}

	// TODO: DeltaTime
	void UpdateCamera(Camera* camera, RenderGroup* renderGroup, World* world)
	{
		if (GlobalInput.keys[KEY_F1].pressedNow &&
			!GlobalInput.keys[KEY_F1].wasPressed)
		{
			camera->debugMode = !camera->debugMode;	
		}

		if (!camera->debugMode)
		{
			if (GlobalInput.mouseButtons[MBUTTON_RIGHT].pressedNow)
			{
 				v2 mousePos;
				f32 speed = 1000.0f;
				mousePos.x = GlobalInput.mouseFrameOffsetX * speed;
				mousePos.y = GlobalInput.mouseFrameOffsetY * speed;
				camera->lastMousePos.x += mousePos.x;
				camera->lastMousePos.y -= mousePos.y;
			}

			if (camera->lastMousePos.y < 95.0f)
			{
				camera->lastMousePos.y = 95.0f;
			}
			else if (camera->lastMousePos.y > 170.0f)
			{
				camera->lastMousePos.y = 170.0f;
			}
		
			f32 scrollSpeed = 5.0f;
		
			i32 frameScrollOffset = GlobalInput.scrollFrameOffset;
			camera->targetDistance -=
				frameScrollOffset * scrollSpeed;
#if 0
			if (camera->targetDistance < 5.0f)
			{
				camera->targetDistance = 5.0f;
			}
			else if (camera->targetDistance > 50.0f)
			{
				camera->targetDistance = 50.0f;
			}
#endif
			camera->latitude = Lerp(camera->latitude, camera->lastMousePos.y,
									camera->latSmoothness);

			camera->longitude = Lerp(camera->longitude, camera->lastMousePos.x,
									 camera->longSmoothness);

			camera->distance = Lerp(camera->distance, camera->targetDistance,
									camera->distSmoothness);

			f32 latitude = ToRadians(camera->latitude);
			f32 longitude = ToRadians(camera->longitude);
			f32 polarAngle = PI_32 - latitude;
			
			f32 x = camera->distance * Sin(polarAngle) * Sin(longitude);
			f32 z = camera->distance * Sin(polarAngle) * Cos(longitude);
			f32 y = camera->distance * Cos(polarAngle);

			camera->pos = V3(x, y, z);
		
			camera->front = -Normalize(camera->pos);

			camera->posWorld = FlipYZ(camera->pos);
			camera->frontWorld = FlipYZ(camera->front);

			camera->lookAt = LookAtLH(camera->pos, V3(0.0f), V3(0.0f, 1.0f, 0.0f));
			camera->invLookAt = camera->lookAt;
			bool inverted = Inverse(&camera->invLookAt);
			//AB_ASSERT(inverted);

			camera->projection = PerspectiveOpenGLLH(camera->fov,
													 camera->aspectRatio,
													 camera->nearPlane,
													 camera->farPlane);
			camera->invProjection = camera->projection;
			inverted = Inverse(&camera->invProjection);
			AB_ASSERT(inverted);

			v3 xAxis = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), camera->front)); 
			v3 yAxis = Cross(camera->front, xAxis);

			camera->up = yAxis;

			v3 posRaw = camera->pos;
			v3 frontRaw = camera->front;
			
			m4x4 rawLookAt = LookAtLH(posRaw, V3(0.0f), V3(0.0f, 1.0f, 0.0f));

			RenderGroupSetCamera(renderGroup,
								 // Normallization?
								 frontRaw, 
								 posRaw,
								 &rawLookAt);
			renderGroup->projectionMatrix =
				PerspectiveOpenGLLH(camera->fov,
									camera->aspectRatio,
									camera->nearPlane,
									camera->farPlane);

			v2 normMousePos;
			normMousePos.x = 2.0f *	GlobalInput.mouseX - 1.0f;
			normMousePos.y = 2.0f *	GlobalInput.mouseY - 1.0f;
			v4 mouseClip = V4(normMousePos, 1.0f, 0.0f);
			v4 mouseView = MulM4V4(camera->invProjection, mouseClip);
			mouseView = V4(mouseView.xy, 1.0f, 0.0f);
			v3 mouseWorld = MulM4V4(camera->invLookAt, mouseView).xyz;
			mouseWorld = Normalize(mouseWorld);
			camera->mouseRay = mouseWorld;
			camera->mouseRayWorld = FlipYZ(mouseWorld);
		}
		else
		{
			DEBUG_OVERLAY_PUSH_SLIDER("Debug camera speed:", &camera->debugSpeed,
									  0.0f, 10.0f);
			if (GlobalInput.keys[KEY_W].pressedNow)
			{
				camera->debugPos += camera->debugFront * GlobalAbsDeltaTime
					* camera->debugSpeed;
			}
			if (GlobalInput.keys[KEY_S].pressedNow)
			{
				camera->debugPos -= camera->debugFront * GlobalAbsDeltaTime
					* camera->debugSpeed;
			}
			if (GlobalInput.keys[KEY_A].pressedNow)
			{
				v3 right = Normalize(Cross(camera->debugFront, { 0, 1, 0 }));
				camera->debugPos += right * GlobalAbsDeltaTime
					* camera->debugSpeed;
			}
			if (GlobalInput.keys[KEY_D].pressedNow)
			{
				v3 right = Normalize(Cross(camera->debugFront, { 0, 1, 0 }));
				camera->debugPos -= right * GlobalAbsDeltaTime
					* camera->debugSpeed;
			}

			if (GlobalInput.keys[KEY_TAB].pressedNow &&
				!GlobalInput.keys[KEY_TAB].wasPressed)
			{
				camera->debugCursorCaptured = !camera->debugCursorCaptured;
			}

			if (camera->debugCursorCaptured)
			{
				camera->debugPitch += GlobalInput.mouseFrameOffsetY;
				camera->debugYaw -= GlobalInput.mouseFrameOffsetX;	
			}

			
			if (camera->debugPitch > 89.0f)
				camera->debugPitch = 89.0f;
			if (camera->debugPitch < -89.0f)
				camera->debugPitch = -89.0f;

			camera->debugFront.x = Cos(ToRadians(camera->debugPitch))
				* Cos(ToRadians(camera->debugYaw));
			camera->debugFront.y = Sin(ToRadians(camera->debugPitch));
			camera->debugFront.z = Cos(ToRadians(camera->debugPitch))
				* Sin(ToRadians(camera->debugYaw));
			camera->debugFront = Normalize(camera->debugFront);

			v3 pos = camera->debugPos;
			v3 front = camera->debugFront;
			
			camera->debugLookAt = LookAtLH(pos,
										AddV3V3(pos, front),
										V3(0.0f, 1.0f, 0.0f));
		
			RenderGroupSetCamera(renderGroup,
								 front,
								 pos,
								 &camera->debugLookAt);
		}

	}
}

#if 0
	void
	DrawChunkInstanced(World* world, RenderGroup* renderGroup,
					   AssetManager* assetManager, WorldPosition origin,
					   Chunk* chunk, u32 selectedTileX, u32 selectedTileY)
	{
		if (chunk)
		{
			v2 offset = V2(0.0f);
			offset.x = (chunk->coordX - origin.chunkX) * world->chunkSizeUnits;
			offset.x -= origin.offset.x;
			offset.y = (chunk->coordY - origin.chunkY) * world->chunkSizeUnits;
			offset.y -= origin.offset.y;

			u32 tilesDrawn = 0;
			for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
			{
				for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
				{
					TerrainTile _tile = 
						FindTileBeforeFirstGapInCell(world, chunk,
													 {tileX, tileY, 0});
					TerrainTileData tile;
					u32 tileZ;
					if (!_tile.data.type)
					{
						tile = GetTerrainTile(chunk, tileX, tileY, WORLD_CHUNK_DIM_TILES - 1);
						tileZ = WORLD_CHUNK_DIM_TILES - 1;
					}
					else
					{
						tile = _tile.data;
						tileZ = _tile.coord.z;
					}

					if (tile.type)
					{
						tilesDrawn++;
						f32 pX = offset.x + tileX * world->tileSizeInUnits;
						f32 pZ = offset.y + tileY * world->tileSizeInUnits;
						//pX += world->tileSizeInUnits * 0.5f;
						//pZ += world->tileSizeInUnits * 0.5f;				

						v3 color = {};
						f32 pY = tileZ * world->tileSizeInUnits;
						//pY -= world->tileRadiusInUnits;

						if (selectedTileX == tileX &&
							selectedTileY == tileY)
						{
							color = V3(0.8f, 0.0f, 0.0f);
						}
						else
						{
							switch (tile.type)
							{
							case TERRAIN_TYPE_CLIFF:
							{
								color = V3(0.7f, 0.7f, 0.7f);
							} break;
							case TERRAIN_TYPE_GRASS:
							{
								color = V3(0.1f, 0.7f, 0.2f);
							} break;
							case TERRAIN_TYPE_WATER:
							{
								color = V3(0.0f, 0.0f, 0.8f);
							} break;
							INVALID_DEFAULT_CASE();
							}							
						}


						DrawDebugCubeInstanced(renderGroup,
											   assetManager,
											   V3(pX, pY, pZ),
											   world->tileSizeInUnits * 0.5f,
											   color);		
					}
				}
			}
			DEBUG_OVERLAY_TRACE(tilesDrawn);
		}
	}

	
	void
	DrawWorldInstanced(World* world, RenderGroup* renderGroup,
					   AssetManager* assetManager,
					   WorldPosition origin, TileWorldPos* selectedTile)
	{
		// TODO: Pass this area as parameters	
		i32 beginX = origin.chunkX - 1;
		i32 beginY = origin.chunkY - 1;

		i32 endX = origin.chunkX + 2;
		i32 endY = origin.chunkY + 2;

		RenderCommandBeginDebugCubeInctancing begCommand = {};
		begCommand.blendMode = BLEND_MODE_OPAQUE;

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_BEGIN_DEBUG_CUBE_INSTANCING,
							   (void*)(&begCommand));

		for (i32 y = beginY; y < endY; y++)
		{
			for (i32 x = beginX; x < endX; x++)
			{
				u32 selectedTileX = INVALID_TILE_COORD;
				u32 selectedTileY = INVALID_TILE_COORD;
				if (selectedTile &&
					selectedTile->chunkX == x &&
					selectedTile->chunkY == y)
				{
					selectedTileX = selectedTile->tileX;
					selectedTileY = selectedTile->tileY;
				}
				Chunk* chunk = GetChunk(world, x, y);
				DrawChunkInstanced(world, renderGroup,
								   assetManager, origin,
								   chunk, selectedTileX, selectedTileY);
			}
		}

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING,
							   nullptr);
	}
#endif
