#include "Camera.h"

namespace AB
{
	void
	DrawChunkInstanced(World* world, RenderGroup* renderGroup,
					   AssetManager* assetManager, WorldPosition origin,
					   Chunk* chunk)
	{
		if (chunk)
		{
			v2 offset = V2(0.0f);
			offset.x = (chunk->coordX - origin.chunkX) * world->chunkSizeUnits;
			offset.x -= origin.offset.x;
			offset.y = (chunk->coordY - origin.chunkY) * world->chunkSizeUnits;
			offset.y -= origin.offset.y;

			for (u32 tileY = 0; tileY < WORLD_CHUNK_DIM_TILES; tileY++)
			{
				for (u32 tileX = 0; tileX < WORLD_CHUNK_DIM_TILES; tileX++)
				{
					TerrainType type = GetTerrainTile(chunk, tileX, tileY);
					if (type)
					{
						f32 pX = offset.x + tileX * world->tileSizeInUnits;
						f32 pZ = offset.y + tileY * world->tileSizeInUnits;

						pX *= world->unitsToRaw;
						pZ *= world->unitsToRaw;

						v3 color = {};
						f32 pY = 0.0f;
						switch (type)
						{
						case TERRAIN_TYPE_CLIFF:
						{
							color = V3(0.7f, 0.7f, 0.7f);
							pY = 2.0f;
						} break;
						case TERRAIN_TYPE_GRASS:
						{
							color = V3(0.1f, 0.7f, 0.2f);
						} break;
						case TERRAIN_TYPE_WATER:
						{
							color = V3(0.0f, 0.0f, 0.8f);
							pY = -1.0f;
						} break;
						INVALID_DEFAULT_CASE();
						}

						DrawDebugCubeInstanced(renderGroup,
											   assetManager,
											   V3(pX, pY, pZ),
											   world->tileSizeRaw * 0.5f,
											   color);		
					}
				}
			}
		}
	}

	
	void
	DrawWorldInstanced(World* world, RenderGroup* renderGroup,
					   AssetManager* assetManager,
					   WorldPosition origin)
	{
		// TODO: Pass this area as parameters	
		i32 beginX = origin.chunkX - 1;
		i32 beginY = origin.chunkY - 1;

		i32 endX = origin.chunkX + 2;
		i32 endY = origin.chunkY + 2;

		DEBUG_OVERLAY_TRACE_VAR(beginX);
		DEBUG_OVERLAY_TRACE_VAR(beginY);
		DEBUG_OVERLAY_TRACE_VAR(endX);
		DEBUG_OVERLAY_TRACE_VAR(endY);
		
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
				Chunk* chunk = GetChunk(world, x, y);
				DrawChunkInstanced(world, renderGroup,
								   assetManager, origin,
								   chunk);
			}
		}

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING,
							   nullptr);
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
			Square(GlobalGameDeltaTime) + 
			camera->targetWorldVelocity *
			GlobalGameDeltaTime;

		WorldPosition newPos = {};
		v2 newVelocity = {};

		newPos = ChangeWorldPosition(world, camera->targetWorldPos, movementDelta);
		newVelocity = camera->targetWorldVelocity;

		newVelocity = newVelocity +
			acceleration * GlobalGameDeltaTime;

		camFrameOffset = WorldPosDiff(world, newPos, camera->targetWorldPos);
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
				mousePos.x = GlobalInput.mouseFrameOffsetX;
				mousePos.y = GlobalInput.mouseFrameOffsetY;
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
			camera->targetDistance -= frameScrollOffset * scrollSpeed;

			if (camera->targetDistance < 5.0f)
			{
				camera->targetDistance = 5.0f;
			}
			else if (camera->targetDistance > 50.0f)
			{
				camera->targetDistance = 50.0f;
			}

			camera->latitude = Lerp(camera->latitude, camera->lastMousePos.y,
									camera->latSmoothness);

			camera->longitude = Lerp(camera->longitude, camera->lastMousePos.x,
									 camera->longSmoothness);

			camera->distance = Lerp(camera->distance, camera->targetDistance,
									camera->distSmoothness);

			f32 latitude = ToRadians(camera->latitude);
			f32 longitude = ToRadians(camera->longitude);
			f32 polarAngle = PI_32 - latitude;

			f32 z = camera->distance * Sin(polarAngle) * Cos(longitude);
			f32 x = camera->distance * Sin(polarAngle) * Sin(longitude);
			f32 y = camera->distance * Cos(polarAngle);

			camera->pos = V3(x, y, z);
		
			camera->front = -Normalize(camera->pos);

			v3 pos = camera->pos * world->unitsToRaw;
			v3 front = camera->front * world->unitsToRaw;

			m4x4 rawLookAt = LookAtLH(pos,
									  AddV3V3(pos, front),
									  V3(0.0f, 1.0f, 0.0f));
		
			RenderGroupSetCamera(renderGroup,
								 // Normallization?
								 front, 
								 pos,
								 &rawLookAt);


			renderGroup->projectionMatrix =
				PerspectiveOpenGLLH(camera->fov,
							  camera->aspectRatio,
							  camera->nearPlane * world->unitsToRaw,
							  camera->farPlane * world->unitsToRaw);
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
				camera->debugPos -= right * GlobalAbsDeltaTime
					* camera->debugSpeed;
			}
			if (GlobalInput.keys[KEY_D].pressedNow)
			{
				v3 right = Normalize(Cross(camera->debugFront, { 0, 1, 0 }));
				camera->debugPos += right * GlobalAbsDeltaTime
					* camera->debugSpeed;
			}

			camera->debugPitch += GlobalInput.mouseFrameOffsetY;
			camera->debugYaw += GlobalInput.mouseFrameOffsetX;
			
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

			v3 pos = camera->debugPos * world->unitsToRaw;
			v3 front = camera->debugFront * world->unitsToRaw;
			
			camera->debugLookAt = LookAtRH(pos,
										AddV3V3(pos, front),
										V3(0.0f, 1.0f, 0.0f));
		
			RenderGroupSetCamera(renderGroup,
								 front,
								 pos,
								 &camera->debugLookAt);
		}

	}
}