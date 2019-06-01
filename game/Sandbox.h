#pragma once
#include "Tilemap.h"
#include "Camera.h"

namespace AB
{
	struct World
	{
		Tilemap tilemap;
	};

	enum EntityResidence
	{
		ENTITY_RESIDENCE_NOT_EXIST = 0,
		ENTITY_RESIDENCE_HIGH,
		ENTITY_RESIDENCE_LOW,
		ENTITY_RESIDENCE_DORMANT,
	};

	enum EntityType
	{
		ENTITY_TYPE_BODY,
		ENTITY_TYPE_WALL
	};
	
	struct HighEntity
	{
		v2 pos;
		v2 velocity;
	};

	struct LowEntity
	{
		
	};

	struct DormantEntity
	{
		EntityType type;
		TilemapPosition tilemapPos;
		f32 accelerationAmount;
		v2 size;
		v3 color;
		f32 friction;
	};

	struct Entity
	{
		EntityResidence residence;
		HighEntity* high;
		LowEntity* low;
		DormantEntity* dormant;
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
		//TilemapPosition playerP;
		//v2 playerSpeed;
		World* world;
		v3 dirLightOffset;
		//FrustumVertices camFrustum;
		u32 entity;
		u32 entity1;

		u32 movingEntities[128];

		u32 entityCount;

#define MAX_ENTITIES 1024
		EntityResidence entityResidence[MAX_ENTITIES];
		HighEntity highEntities[MAX_ENTITIES];
		LowEntity lowEntities[MAX_ENTITIES];
		DormantEntity dormantEntities[MAX_ENTITIES];
	};

	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  GameState* gameState,
			  AssetManager* assetManager);
	
	void Render(GameState* gameState,
				AssetManager* assetManager,
				Renderer* renderer);
}
