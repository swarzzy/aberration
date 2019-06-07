#include "Camera.h"

namespace AB
{

	
	void DrawTileInstanced(Tilemap* tilemap, RenderGroup* renderGroup,
						   AssetManager* assetManager, TilemapPosition origin,
						   TilemapPosition tilePos)
	{
		u32 tileValue = GetTileValue(tilemap, tilePos.tileX, tilePos.tileY);
		if (tileValue)
		{
			i32 relTileOffsetX = tilePos.tileX - origin.tileX;
			f32 relOffsetX = tilePos.offset.x - origin.offset.x;
			i32 relTileOffsetY = tilePos.tileY - origin.tileY;
			f32 relOffsetY = tilePos.offset.y - origin.offset.y;
		
			f32 pX = relTileOffsetX * tilemap->tileSizeInUnits + relOffsetX;
			f32 pZ = relTileOffsetY * tilemap->tileSizeInUnits + relOffsetY;

			pX *= tilemap->unitsToRaw;
			pZ *= tilemap->unitsToRaw;

			v3 color;
			f32 pY = 0.0f;
			// TODO: Temporary code
			{
				if (tileValue == 3)
				{
					pY = 0.0f;
				}
				else
				{
					pY = 0.0f;
				}
				ChunkPosition chunkTile = GetChunkPosition(tilemap, tilePos.tileX,
														   tilePos.tileY);

				Chunk* chunk = GetChunk(tilemap, chunkTile.chunkX,
										chunkTile.chunkY);
				// TODO: Temporary
				if (chunk == nullptr)
				{
					return;
				}
				color = GetTileColor(tilemap,  chunk, chunkTile.tileInChunkX,
									 chunkTile.tileInChunkY);
			}

			DrawDebugCubeInstanced(renderGroup,
								   assetManager,
								   V3(pX, pY, pZ),
								   tilemap->tileSizeRaw * 0.5f,
								   color);
		}
	}
	
	void DrawWorldWholeInstanced(Tilemap* tilemap, RenderGroup* renderGroup,
								 AssetManager* assetManager,
								 TilemapPosition origin)
	{
		
		u32 beginX = origin.tileX - 64;
		u32 beginY = origin.tileY - 64;

		u32 endX = origin.tileX + 64;
		u32 endY = origin.tileY + 64;

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
								  TilemapPosition{x, y});
					
			}
		}

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING,
							   nullptr);
	}

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

	void
	DoMovement(Tilemap* tilemap, TilemapPosition begPos, v2 begVelocity, v2 delta,
			   TilemapPosition* newPos, v2* newVelocity)
	{

		*newVelocity = begVelocity;
		
		TilemapPosition rawNewPos = begPos;
		rawNewPos = OffsetTilemapPos(tilemap, rawNewPos, delta);

		v2 colliderSize = V2(tilemap->tileSizeInUnits,
							 tilemap->tileSizeInUnits);

		u32 colliderTileWidth = Ceil(colliderSize.x / tilemap->tileSizeInUnits);
		u32 colliderTileHeight = Ceil(colliderSize.y / tilemap->tileSizeInUnits);

		u32 minTileY = MINIMUM(begPos.tileY, rawNewPos.tileY);
		u32 maxTileY = MAXIMUM(begPos.tileY, rawNewPos.tileY);
		u32 minTileX = MINIMUM(begPos.tileX, rawNewPos.tileX);
		u32 maxTileX = MAXIMUM(begPos.tileX, rawNewPos.tileX);

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
			auto testingPos = begPos;//OffsetTilemapPos(tilemap, sandbox->playerP,

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
						v2 relOldPos = TilemapPosDiff(tilemap, &testingPos,
													  &testTilePos);

						if (TestWall(minCorner.x, relOldPos.x,
									 relOldPos.y, delta.x,
									 delta.y,
									 minCorner.y, maxCorner.y, &tMin))
						{
							wallNormal = V2(1.0f, 0.0f);
							hit = true;
						}
						
						if(TestWall(maxCorner.x, relOldPos.x,
									relOldPos.y, delta.x,
									delta.y,
									minCorner.y, maxCorner.y, &tMin))
						{
							wallNormal = V2(-1.0f, 0.0f);
							hit = true;
						}
						if(TestWall(minCorner.y, relOldPos.y,
									relOldPos.x, delta.y,
									delta.x,
									minCorner.x, maxCorner.x, &tMin))
						{
							wallNormal = V2(0.0f, 1.0f);
							hit = true;
						}
						if(TestWall(maxCorner.y, relOldPos.y,
									relOldPos.x, delta.y,
									delta.x,
									minCorner.x, maxCorner.x, &tMin))
						{
							wallNormal = V2(0.0f, -1.0f);
							hit = true;
						}
					}
				}
			}
			v2 frameOffset = delta * tMin;
			*newPos = OffsetTilemapPos(tilemap, begPos,
									   frameOffset);
			if (hit)
			{
				tRemaining -= tMin * tRemaining;
				*newVelocity = *newVelocity -
					Dot(*newVelocity, wallNormal) * wallNormal;
				delta *= 1.0f - tMin;
				delta = delta -
					Dot(delta, wallNormal) * wallNormal;				
			}
			else
			{
				break;
			}
		}

		//sandbox->playerSpeed = Reflect(sandbox->playerSpeed,  wallNormal);

	}

	v2 MoveCameraTarget(Camera* camera, Tilemap* tilemap)
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

		TilemapPosition newPos = {};
		v2 newVelocity = {};
		DoMovement(tilemap,
				   camera->targetWorldPos,
				   camera->targetWorldVelocity,
				   movementDelta,
				   &newPos,
				   &newVelocity);

		newVelocity = newVelocity +
			acceleration * GlobalGameDeltaTime;

		camFrameOffset = TilemapPosDiff(tilemap, &newPos, &camera->targetWorldPos);
		camera->targetWorldPos = newPos;
		camera->targetWorldVelocity = newVelocity;

		return camFrameOffset;
	}

	// TODO: DeltaTime
	void UpdateCamera(Camera* camera, RenderGroup* renderGroup, Tilemap* tilemap)
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

			v3 pos = camera->pos * tilemap->unitsToRaw;
			v3 front = camera->front * tilemap->unitsToRaw;

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
							  camera->nearPlane * tilemap->unitsToRaw,
							  camera->farPlane * tilemap->unitsToRaw);
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

			v3 pos = camera->debugPos * tilemap->unitsToRaw;
			v3 front = camera->debugFront * tilemap->unitsToRaw;
			
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
