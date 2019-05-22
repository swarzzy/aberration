#pragma once

namespace AB
{	
	struct FrustumGroundProjection
	{
		v2 lt;
		v2 lb;
		v2 rt;
		v2 rb;
	};

	struct FrustumGroundAABB
	{
		i32 minX, minY;
		i32 maxX, maxY;
	};

	struct Camera
	{
		// In tiles
		b32 debugMode;
		v3 debugPos;
		v3 debugFront;
		f32 debugPitch;
		f32 debugYaw;
		f32 debugSpeed;
		m4x4 debugLookAt;
		
		f32 longSmoothness;
		f32 latSmoothness;
		f32 distSmoothness;
		f32 longitude;
		f32 latitude;
		v3 pos;
		v3 front;
		//v3 target;
		TilemapPosition targetWorldPos;
		v2 targetWorldVelocity;
		f32 distance;
		v2 lastMousePos;
		f32 targetDistance;

		f32 fov;
		f32 aspectRatio;
		f32 nearPlane;
		f32 farPlane;

		// TODO: Not store this. They only used for generating far frustum plane
		m4x4 cullingProjection;
		m4x4 cullingLookAt;


		f32 cullPosAdjust;
		FrustumGroundProjection frustumGroundPoints;
		FrustumGroundAABB frustumGroundAABB;
	};

	union FrustumVertices
	{
		struct
		{
			v3 nearLeftTop;
			v3 nearLeftBottom;
			v3 nearRightTop;
			v3 nearRightBottom;
			v3 farLeftTop;
			v3 farLeftBottom;
			v3 farRightTop;
			v3 farRightBottom;
		};
		struct
		{
			v3 vertices[8];
		};
	};

	
	b32 IsTileInsideFrustumProj(v2 relTilePos, FrustumGroundProjection* fr);
	void DrawTileInstanced(Tilemap* tilemap, RenderGroup* renderGroup,
						   AssetManager* assetManager, TilemapPosition origin,
						   TilemapPosition tile, FrustumGroundProjection* frustum);
	
	void DrawWorldInstancedMinMax(Tilemap* tilemap, RenderGroup* renderGroup,
								  AssetManager* assetManager,
								  TilemapPosition origin,
								  FrustumGroundAABB bbox,
								  FrustumGroundProjection* frustumProj);

	void GenFrustumVertices(Tilemap* tilemap, Camera* cam, FrustumVertices* out);
	
	b32 RaycastToGround(const Tilemap* tilemap, v3 from, v3 dir, Plane* farPlane, 
						i32* hitTileOffsetX, i32* hitTileOffsetY);

	void GenFrustumCullingData(Camera* camera, Tilemap* tilemap,
							   FrustumGroundAABB* bbox,
							   FrustumGroundProjection* proj);

	b32 TestWall(f32 wallX, f32 relPlayerX, f32 relPlayerY,
				 f32 playerDeltaX, f32 playerDeltaY,
				 f32 minCornerY, f32 maxCornerY, f32* tMin);

	void DoMovement(Tilemap* tilemap, TilemapPosition begPos,
					v2 begVelocity, v2 delta,
					TilemapPosition* newPos, v2* newVelocity);

	void MoveCameraTarget(Camera* camera, Tilemap* tilemap);

	void UpdateCamera(Camera* camera, RenderGroup* renderGroup, Tilemap* tilemap);
}
