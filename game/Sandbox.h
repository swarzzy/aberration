#pragma once
#include "Tilemap.h"
#include "Tilemap.cpp"

namespace AB
{
	struct AnnoCamera
	{
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
		v3 target;
		f32 distance;
		v2 lastMousePos;
		f32 targetDistance;
		m4x4 lookAt;
	};

	struct MouseInputState
	{
		b32 mouseCaptured;
		f32 lastMouseX;
		f32 lastMouseY;
	};

	struct World
	{
		Tilemap tilemap;
	};

	struct Sandbox
	{
		RenderGroup* renderGroup;
		i32 mansionMeshHandle;
		i32 planeMeshHandle;
		f32 gamma;
		f32 exposure;
		AnnoCamera camera;
		MouseInputState inputState;
		DirectionalLight dirLight;
		TilemapPosition playerP;
		v2 playerSpeed;
		World* world;
		v3 dirLightOffset;
		FrustumVertices camFrustum;
	};
	


	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  Sandbox* sandbox,
			  AssetManager* assetManager);
	
	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer);
}
