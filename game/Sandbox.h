#pragma once
#include "World.h"
#include "Camera.h"

namespace AB
{
	enum MouseDragAxis : u32
	{
		MOUSE_DRAG_AXIS_NULL,
		MOUSE_DRAG_AXIS_X,
		MOUSE_DRAG_AXIS_Y,
		MOUSE_DRAG_AXIS_Z
	};
	
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
		MouseDragAxis dragAxis;
		f32 yDragSpeed;
		//EntityResidence entityResidence[MAX_LOW_ENTITIES];
		TileWorldPos selectedTile;
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
