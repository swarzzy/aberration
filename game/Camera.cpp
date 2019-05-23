#include "Camera.h"

namespace AB
{

	static inline u32 SafeAdd(u32 a, i32 b);

	static inline u32 SafeSub(u32 a, i32 b)
	{
		u32 result;
		if (b < 0)
		{
			result = SafeAdd(a, -b);
		}
		else
		{
			if ((u32)b > a)
			{
				result = 0;
			}
			else
			{
				result = a - b;
			}
		}
		
		return result;
	}

	static inline u32 SafeAdd(u32 a, i32 b)
	{
		u32 result;
		if (b < 0)
		{
			result = SafeSub(a, -b);
		}
		else
		{
			if (a + b < a)
			{
				result = 0xffffffff;
			}
			else
			{
				result = a + b;
			}
		}
		return result;
	}
			
	b32 IsTileInsideFrustumProj(v2 relTilePos, FrustumGroundProjection* fr)
	{
		// https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located
		b32 result = false;
		// TODO: Inverse frustum Z OR maybe just inverse world Z
		//relTilePos.y *= -1.0f;
		b32 d1 = ((relTilePos.x - fr->lb.x) * (fr->lt.y - fr->lb.y) -
				  (relTilePos.y - fr->lb.y) * (fr->lt.x - fr->lb.x)) < 0 ? 0 : 1;

		b32 d2 = ((relTilePos.x - fr->lt.x) * (fr->rt.y - fr->lt.y) -
				  (relTilePos.y - fr->lt.y) * (fr->rt.x - fr->lt.x)) < 0 ? 0 : 1;

		b32 d3 = ((relTilePos.x - fr->rt.x) * (fr->rb.y - fr->rt.y) -
				  (relTilePos.y - fr->rt.y) * (fr->rb.x - fr->rt.x)) < 0 ? 0 : 1;

		b32 d4 = ((relTilePos.x - fr->rb.x) * (fr->lb.y - fr->rb.y) -
				  (relTilePos.y - fr->rb.y) * (fr->lb.x - fr->rb.x)) < 0 ? 0 : 1;

		result = d1 && d2 && d3 && d4;
		
		return result;
	}

	void DrawTileInstanced(Tilemap* tilemap, RenderGroup* renderGroup,
						   AssetManager* assetManager, TilemapPosition origin,
						   TilemapPosition tile, FrustumGroundProjection* frustum)
	{
		ChunkPosition chunkOrigin = GetChunkPosition(tilemap, origin.tileX,
													 origin.tileY);
		ChunkPosition chunkTile = GetChunkPosition(tilemap, tile.tileX,
												   tile.tileY);

		u32 tileValue = GetTileValue(tilemap, tile.tileX, tile.tileY);
		if (tileValue)
		{
			i32 chunkDx = chunkTile.chunkX - chunkOrigin.chunkX;
			i32 chunkDy = chunkTile.chunkY - chunkOrigin.chunkY;
			i32 tileDx = chunkTile.tileInChunkX - chunkOrigin.tileInChunkX;
			i32 tileDy = chunkTile.tileInChunkY - chunkOrigin.tileInChunkY;
			i32 tileOffsetX = chunkDx * (i32)tilemap->chunkSizeInTiles + tileDx;
			i32 tileOffsetY = chunkDy * (i32)tilemap->chunkSizeInTiles + tileDy;

			f32 x = tileOffsetX * tilemap->tileSizeInUnits
				- origin.offset.x - tilemap->tileRadiusInUnits;
			f32 y = (tileOffsetY * tilemap->tileSizeInUnits
				- origin.offset.y - tilemap->tileRadiusInUnits);

			// TODO: This is temporary
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
			
			if (IsTileInsideFrustumProj(V2(x, y), frustum))
			{

				v3 renderPos = V3(x * tilemap->unitsToRaw,
								  yOffset,
								  -y * tilemap->unitsToRaw);
				
				DrawDebugCubeInstanced(renderGroup,
									   assetManager,
									   renderPos,
									   tilemap->tileSizeRaw * 0.5f,
									   color);
			}
		}
	}

	void DrawWorldInstancedMinMax(Tilemap* tilemap, RenderGroup* renderGroup,
								  AssetManager* assetManager,
								  TilemapPosition origin,
								  FrustumGroundAABB bbox,
								  FrustumGroundProjection* frustumProj)
	{
		u32 beginX = SafeAdd(origin.tileX, bbox.minX);// - offsetX;
		u32 beginY = SafeAdd(origin.tileY, bbox.minY);// - offsetY;

		u32 endX = SafeAdd(origin.tileX, bbox.maxX);// - offsetX;
		u32 endY = SafeAdd(origin.tileY, bbox.maxY);// - offsetY;

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
								  TilemapPosition{x, y}, frustumProj);
					
			}
		}

		RenderGroupPushCommand(renderGroup,
							   assetManager,
							   RENDER_COMMAND_END_DEBUG_CUBE_INSTANCING,
							   nullptr);
	}

	// TODO: Merge with projection mtx generation
	void GenFrustumVertices(Tilemap* tilemap, Camera* camera, FrustumVertices* out)
	{
		v3 camPos = camera->pos;// * tilemap->unitsToRaw;

		m3x3 rot = M3x3(camera->cullingLookAt);
		v3 axisX = V3(rot._11, rot._12, rot._13);
		v3 axisY = V3(rot._21, rot._22, rot._23);
		v3 axisZ = V3(rot._31, rot._32, rot._33);

		v3 nCenter = -axisZ * camera->nearPlane;
		v3 fCenter = -axisZ * camera->farPlane;

		f32 e = Tan(ToRadians(camera->fov) * 0.5f);
		f32 nExtY = e * camera->nearPlane;
		f32 nExtX = nExtY * camera->aspectRatio;
		f32 fExtY = e * camera->farPlane;
		f32 fExtX = fExtY * camera->aspectRatio;

		out->nearLeftBottom =
			(nCenter - axisX * nExtX - axisY * nExtY + camPos);
		out->nearLeftTop =
			(nCenter - axisX * nExtX + axisY * nExtY + camPos);
		out->nearRightTop =
			(nCenter + axisX * nExtX + axisY * nExtY + camPos);
		out->nearRightBottom =
			(nCenter + axisX * nExtX - axisY * nExtY + camPos);
		out->farLeftBottom =
			(fCenter - axisX * fExtX - axisY * fExtY + camPos);
		out->farLeftTop =
			(fCenter - axisX * fExtX + axisY * fExtY + camPos);
		out->farRightTop =
			(fCenter + axisX * fExtX + axisY * fExtY + camPos);
		out->farRightBottom =
			(fCenter + axisX * fExtX - axisY * fExtY + camPos);
	}

	
	b32 RaycastToGround(const Tilemap* tilemap, v3 from, v3 dir, Plane* farPlane, 
						i32* hitTileOffsetX, i32* hitTileOffsetY)
	{
		AB_ASSERT(Length(dir) >= FLOAT_EPS);
		// NOTE: Returns offset relative to from
		b32 result = false;
		f32 t = 0.0f;

		// NOTE: Raycasting against far plane
		f32 tToFarPlane = (-farPlane->a * from.x - farPlane->b
						   * from.y - farPlane->c * from.z - farPlane->d)
			/ (farPlane->a * dir.x + farPlane->b * dir.y
			   + farPlane->c * dir.z);

		if (dir.y < 0.0f)
		{
			// NOTE: Raycasting to ground plane
			f32 tToGround =  -from.y / dir.y;

			if (tToGround < tToFarPlane)
			{
				t = tToGround;
				result = true;
			}
			else
			{
				t = tToFarPlane;
				result = true;
			}
		}
		else
		{
			t = tToFarPlane;
			result = true;
		}
		
		f32 x = from.x + dir.x * t;
		f32 z = from.z + dir.z * t;
		v3 intersectionCoord = V3(x, 0.0f, z);
		i32 tileOffsetX = TruncF32I32(intersectionCoord.x /
									  tilemap->tileSizeInUnits);
		i32 tileOffsetY = -TruncF32I32(intersectionCoord.z /
									   tilemap->tileSizeInUnits);
		*hitTileOffsetX = tileOffsetX;
		*hitTileOffsetY = tileOffsetY;

		return result;
	}


	void GenFrustumCullingData(Camera* camera, Tilemap* tilemap,
							   FrustumGroundAABB* bbox,
							   FrustumGroundProjection* proj)
	{		
		FrustumVertices camFV = {};
		GenFrustumVertices(tilemap, camera, &camFV);

		// NOTE: Vectors from frustum
		v3 lt = camFV.farLeftTop - camFV.nearLeftTop;
		v3 lb = camFV.farLeftBottom - camFV.nearLeftBottom;
		v3 rb = camFV.farRightBottom - camFV.nearRightBottom;
		v3 rt = camFV.farRightTop - camFV.nearRightTop;

		Plane farPlane = {};

		m4x4 pMtx = MulM4M4(camera->cullingProjection, camera->cullingLookAt);
		farPlane.a = pMtx._41 - pMtx._31;
		farPlane.b = pMtx._42 - pMtx._32;
		farPlane.c = pMtx._43 - pMtx._33;
		farPlane.d = pMtx._44 - pMtx._34;
		farPlane = Normalize(farPlane);

		v3 cullPos = camera->pos * camera->cullPosAdjust;
		
		i32 ltTileX = 0;
		i32 ltTileY = 0;
		RaycastToGround(tilemap, cullPos,
						lt, &farPlane, &ltTileX, &ltTileY);

		i32 lbTileX = 0;
		i32 lbTileY = 0;
		RaycastToGround(tilemap, cullPos,
						lb, &farPlane, &lbTileX, &lbTileY);

		i32 rtTileX = 0;
		i32 rtTileY = 0;
		RaycastToGround(tilemap, cullPos,
						rt, &farPlane, &rtTileX, &rtTileY);

		i32 rbTileX = 0;
		i32 rbTileY = 0;
		RaycastToGround(tilemap, cullPos,
						rb, &farPlane, &rbTileX, &rbTileY);
		
		bbox->minX = MINIMUM(MINIMUM(ltTileX, lbTileX), MINIMUM(rtTileX, rbTileX));
		bbox->minY = MINIMUM(MINIMUM(ltTileY, lbTileY), MINIMUM(rtTileY, rbTileY));
		bbox->maxX = MAXIMUM(MAXIMUM(ltTileX, lbTileX), MAXIMUM(rtTileX, rbTileX));
		bbox->maxY = MAXIMUM(MAXIMUM(ltTileY, lbTileY), MAXIMUM(rtTileY, rbTileY));

		// Traverse all prefetched chunks and check for frustum - box intersection
		//auto camP = -camera->pos;
		proj->lt.x = ltTileX * tilemap->tileSizeInUnits;
		proj->lt.y = ltTileY * tilemap->tileSizeInUnits;
		proj->lb.x = lbTileX * tilemap->tileSizeInUnits;
		proj->lb.y = lbTileY * tilemap->tileSizeInUnits;
		proj->rt.x = rtTileX * tilemap->tileSizeInUnits;
		proj->rt.y = rtTileY * tilemap->tileSizeInUnits;
		proj->rb.x = rbTileX * tilemap->tileSizeInUnits;
		proj->rb.y = rbTileY * tilemap->tileSizeInUnits;

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

	void MoveCameraTarget(Camera* camera, Tilemap* tilemap)
	{
		v3 _frontDir = V3(camera->front.x, 0.0f, camera->front.z);
		v3 _rightDir = Cross(_frontDir, V3(0.0f, 1.0f, 0.0f));
		// TODO: Is that normalization is neccessary?
		_rightDir = Normalize(_rightDir);

		// NOTE: Neagating Z because tilemap coords have Y axis pointing down-to-up
		// while right handed actual world coords have it
		// pointing opposite directhion.
		v2 frontDir = V2(_frontDir.x, -_frontDir.z);
		v2 rightDir = V2(_rightDir.x, -_rightDir.z);

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
		f32 speed = 20.0f;
		acceleration *= speed;

		f32 friction = 3.0f;
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

		camera->targetWorldPos = newPos;
		camera->targetWorldVelocity = newVelocity;
	}
	
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
				camera->lastMousePos.x -= mousePos.x;
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

			v3 cullingPos = camera->pos * camera->cullPosAdjust;

			camera->cullingLookAt = LookAtRH(cullingPos,
											 AddV3V3(cullingPos, camera->front),
											 V3(0.0f, 1.0f, 0.0f));

			camera->cullingProjection = PerspectiveOpenGLRH(camera->fov,
													  camera->aspectRatio,
													  camera->nearPlane,
													  camera->farPlane);

			GenFrustumCullingData(camera, tilemap,
								  &camera->frustumGroundAABB,
								  &camera->frustumGroundPoints);

		
			v3 pos = camera->pos * tilemap->unitsToRaw;
			v3 front = camera->front * tilemap->unitsToRaw;

			m4x4 rawLookAt = LookAtRH(pos,
									  AddV3V3(pos, front),
									  V3(0.0f, 1.0f, 0.0f));
		
			RenderGroupSetCamera(renderGroup,
								 // Normallization?
								 front, 
								 pos,
								 &rawLookAt);


			renderGroup->projectionMatrix =
				PerspectiveOpenGLRH(camera->fov,
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
