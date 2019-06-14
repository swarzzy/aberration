#pragma once
#include "World.h"
#include "Camera.h"

namespace AB
{

	struct Gizmos
	{
		Mesh* xAxisMesh;
		Mesh* yAxisMesh;
		Mesh* zAxisMesh;
		u32 selectedIndex;
		v3 camRelPos;
		f32 scale;
		u32 attachedEntityIndex;
		v3 prevMousePos;
		u32 activeAxis;
	};
	
	struct GameState
	{
		RenderGroup* renderGroup;
		i32 treeFoliageHandle;
		i32 treeTrunkHandle;
		i32 xAxisMeshHandle;
		i32 yAxisMeshHandle;
		i32 zAxisMeshHandle;
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
		u32 selectedEntityIndex;
		Gizmos gizmos;
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
