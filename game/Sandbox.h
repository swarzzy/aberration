#pragma once

namespace AB
{
	struct AnnoCamera
	{
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
		struct
		{
			b32 forward;
			b32 back;
			b32 left;
			b32 right;
		} frameMovementFlags;
	};

	struct InputState
	{
		b32 mouseCaptured;
		f32 lastMouseX;
		f32 lastMouseY;
	};

	struct Sandbox
	{
		RenderGroup* renderGroup;
		i32 mansionMeshHandle;
		i32 planeMeshHandle;
		f32 gamma;
		f32 exposure;
		AnnoCamera camera;
		InputState inputState;
		DirectionalLight dirLight;
	};


	void Init(MemoryArena* arena,
			  MemoryArena* tempArena,
			  Sandbox* sandbox,
			  AssetManager* assetManager,
			  InputManager* inputManager);
	
	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer,
				InputManager* inputManager);
}
