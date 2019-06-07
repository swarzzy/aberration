#pragma once
#include "World.h"
#include "Camera.h"

namespace AB
{
	enum EntityResidence
	{
		ENTITY_RESIDENCE_NOT_EXIST = 0,
		ENTITY_RESIDENCE_HIGH,
		ENTITY_RESIDENCE_LOW
	};

	enum EntityType
	{
		ENTITY_TYPE_BODY,
		ENTITY_TYPE_WALL
	};
	struct LowEntity
	{
		EntityType type;
		WorldPosition worldPos;
		f32 accelerationAmount;
		v2 size;
		v3 color;
		f32 friction;
		u32 highIndex;
	};

	// TODO: 
	struct HighEntity
	{
		v2 pos;
		v2 velocity;
		u32 lowIndex;
	};

	struct Entity
	{
		u32 lowIndex;
		HighEntity* high;
		LowEntity* low;
	};


	struct GameState
	{
		RenderGroup* renderGroup;
		i32 mansionMeshHandle;
		i32 planeMeshHandle;
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
#define MOVING_ENTITIES_COUNT 32 // 128 max
		u32 movingEntities[MOVING_ENTITIES_COUNT];


#define MAX_LOW_ENTITIES 4096
#define MAX_HIGH_ENTITIES 512
		EntityResidence entityResidence[MAX_LOW_ENTITIES];
		u32 lowEntityCount;
		u32 highEntityCount;
		HighEntity highEntities[MAX_HIGH_ENTITIES];
		LowEntity lowEntities[MAX_LOW_ENTITIES];
	};

	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  GameState* gameState,
			  AssetManager* assetManager);
	
	void Render(GameState* gameState,
				AssetManager* assetManager,
				Renderer* renderer);
}
