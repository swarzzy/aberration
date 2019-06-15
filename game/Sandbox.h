#pragma once
#include "World.h"
#include "Camera.h"

namespace AB
{
		struct GameState
	{
		RenderGroup* renderGroup;
		i32 treeFoliageHandle;
		i32 treeTrunkHandle;
		Mesh* xAxisMesh;
		Mesh* yAxisMesh;
		Mesh* zAxisMesh;
		f32 gamma;
		f32 exposure;
		Camera camera;
		//MouseInputState inputState;
		DirectionalLight dirLight;
		//WorldPosition playerP;
		//v2 playerSpeed;
		World* world;
		v3 dirLightOffset;
		//FrustumVertices camFrustum;
		u32 entity;
		u32 entity1;
#define MOVING_ENTITIES_COUNT 64 // 128 max
		u32 movingEntities[MOVING_ENTITIES_COUNT];
		LowEntity* selectedEntity;
		b32 dragActive;
		v3 prevDragPos;
		//EntityResidence entityResidence[MAX_LOW_ENTITIES];
	};

	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  GameState* gameState,
			  AssetManager* assetManager);
	
	void Render(MemoryArena* arena,
				GameState* gameState,
				AssetManager* assetManager,
				Renderer* renderer);
}
