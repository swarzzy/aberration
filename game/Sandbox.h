#pragma once
#include "Tilemap.h"
#include "Camera.h"

namespace AB
{
	struct World
	{
		Tilemap tilemap;
	};

	struct Entity
	{
		TilemapPosition pos;
		v2 velocity;
		f32 accelerationAmount;
	};

	struct Sandbox
	{
		RenderGroup* renderGroup;
		i32 mansionMeshHandle;
		i32 planeMeshHandle;
		f32 gamma;
		f32 exposure;
		Camera camera;
		//MouseInputState inputState;
		DirectionalLight dirLight;
		//TilemapPosition playerP;
		//v2 playerSpeed;
		World* world;
		v3 dirLightOffset;
		//FrustumVertices camFrustum;
		Entity entity;
	};

	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  Sandbox* sandbox,
			  AssetManager* assetManager);
	
	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer);
}
