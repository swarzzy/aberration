#pragma once

namespace AB
{	
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
	};

	b32 TestWall(f32 wallX, f32 relPlayerX, f32 relPlayerY,
				 f32 playerDeltaX, f32 playerDeltaY,
				 f32 minCornerY, f32 maxCornerY, f32* tMin);

	void DoMovement(Tilemap* tilemap, TilemapPosition begPos,
					v2 begVelocity, v2 delta,
					TilemapPosition* newPos, v2* newVelocity);

	v2 MoveCameraTarget(Camera* camera, Tilemap* tilemap);

	void UpdateCamera(Camera* camera, RenderGroup* renderGroup, Tilemap* tilemap);
}
