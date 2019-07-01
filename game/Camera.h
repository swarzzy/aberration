#pragma once

namespace AB
{	
	struct Camera
	{
		// In tiles
		b32 debugMode;
		b32 debugCursorCaptured;
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
		// NOTE: Towards camera look direction NOT backwards
		v3 front;
		v3 up;
		// TODO: Make this in units?
		m4x4 projection;
		m4x4 lookAt;
		m4x4 invProjection;
		m4x4 invLookAt;
		//v3 target;
		// NOTE: This is world coordinates in z-is-up space
		WorldPosition targetWorldPos;
		v3 posWorld;
		v3 frontWorld;
		v3 mouseRayWorld;
		// NOTE: In units. Left-handed coords
		v3 mouseRay;
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

	v3 MoveCameraTarget(Camera* camera, World* world);

	void UpdateCamera(Camera* camera, RenderGroup* renderGroup, World* world);
}
