#include "Sandbox.h"

namespace AB
{
	void SubscribeKeyboardEvents(AnnoCamera* camera,
								 InputManager* inputManager,
								 InputState* inputState);

	void UpdateCamera(AnnoCamera* cam, InputManager* inputManager);

		void Init(MemoryArena* arena,
				  MemoryArena* tempArena,
				  Sandbox* sandbox,
				  AssetManager* assetManager,
				  InputManager* inputManager)
	{
		sandbox->renderGroup = AllocateRenderGroup(arena, MEGABYTES(1),
												   256, 32);
		sandbox->renderGroup->projectionMatrix = PerspectiveRH(45.0f,
															   16.0f / 9.0f,
															   0.1f,
															   100.0f);

		BeginTemporaryMemory(tempArena);
		sandbox->mansionMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena,
							   tempArena,
							   "../assets/mansion/mansion.aab");
		sandbox->planeMeshHandle =
			AssetCreateMeshAAB(assetManager,
							   arena, tempArena,
							   "../assets/Plane.aab");
		
		AB_CORE_ASSERT(sandbox->mansionMeshHandle != ASSET_INVALID_HANDLE);
		EndTemporaryMemory(tempArena);

		sandbox->camera = {};
		sandbox->camera.longSmoothness = 0.3f;
		sandbox->camera.latSmoothness = 0.3f;
		sandbox->camera.distSmoothness = 0.3f;
		sandbox->camera.pos = V3(0.0f, 0.0f, -1.0f);
		sandbox->camera.front = V3(0.0, 0.0f, 1.0f);
	
		//SubscribeKeyboardEvents(&sandbox->camera, inputManager,
		//						&sandbox->inputState);

		sandbox->gamma = 2.2f;
		sandbox->exposure = 1.0f;
	}

	void Render(Sandbox* sandbox,
				AssetManager* assetManager,
				Renderer* renderer,
				InputManager* inputManager)
	{
		DEBUG_OVERLAY_PUSH_SLIDER("Gamma", &sandbox->gamma, 0.0f, 3.0f);
		DEBUG_OVERLAY_PUSH_VAR("Gamma", sandbox->gamma);

		DEBUG_OVERLAY_PUSH_SLIDER("Exposure", &sandbox->exposure, 0.0f, 3.0f);
		DEBUG_OVERLAY_PUSH_VAR("Exposure", sandbox->exposure);

		renderer->cc.gamma = sandbox->gamma;
		renderer->cc.exposure = sandbox->exposure;

		UpdateCamera(&sandbox->camera, inputManager);
		RenderGroupSetCamera(sandbox->renderGroup,
							 sandbox->camera.front, sandbox->camera.pos);
		RenderCommandDrawMesh planeCommand = {};
		planeCommand.meshHandle = sandbox->mansionMeshHandle;
		planeCommand.transform.worldMatrix = Identity4();
		planeCommand.blendMode = BLEND_MODE_OPAQUE;

		
		RenderGroupPushCommand(sandbox->renderGroup,
							   assetManager,
							   RENDER_COMMAND_DRAW_MESH,
							   (void*)(&planeCommand));

		f32 ka = sandbox->dirLight.ambient.r;
		f32 kd = sandbox->dirLight.diffuse.r;
	
		DEBUG_OVERLAY_PUSH_SLIDER("Ka", &ka, 0.0f, 1.0f);
		DEBUG_OVERLAY_PUSH_VAR("Ka", ka);

		DEBUG_OVERLAY_PUSH_SLIDER("Kd", &kd, 0.0f, 10.0f);
		DEBUG_OVERLAY_PUSH_VAR("Kd", kd);

		DEBUG_OVERLAY_PUSH_SLIDER("Dir", &sandbox->dirLight.direction,
								  -1.0f, 1.0f);
		DEBUG_OVERLAY_PUSH_VAR("Dir", sandbox->dirLight.direction);
		sandbox->dirLight.direction = Normalize(sandbox->dirLight.direction);

		sandbox->dirLight.ambient = V3(ka);
		sandbox->dirLight.diffuse = V3(kd);

		RenderCommandSetDirLight dirLightCommand = {};
		dirLightCommand.light = sandbox->dirLight;

		RenderGroupPushCommand(sandbox->renderGroup,
							   assetManager,
							   RENDER_COMMAND_SET_DIR_LIGHT,
							   (void*)(&dirLightCommand));

		RendererRender(renderer, assetManager, sandbox->renderGroup);
		RenderGroupResetQueue(sandbox->renderGroup);
	}
#if 0
	void UpdateInput(AnnoCamera* camera)
	{
		i32 forwardMovement = 0;
		if (InputMouseButtonIsDown(KeyboardKey::W))
		{
			forwardMovement += 1;
		}
		if (InputMouseButtonIsDown(KeyboardKey::S))
		{
			 forwardMovement -= 1;
		}
	}
#endif
#if 0
	void SubscribeKeyboardEvents(AnnoCamera* camera,
								 InputManager* inputManager,
								 InputState* inputState)
	{
		auto CameraMovementCallback = [](AB::Event e)
			{
				int32 action = -1;
				if (e.type == AB::EVENT_TYPE_KEY_PRESSED)
				{
					action = 1;
				}
				else if (e.type == AB::EVENT_TYPE_KEY_RELEASED)
				{
					action = 0;
				}
	
				if (action != -1)
				{
					switch (e.key_event.key)
					{
					case AB::KeyboardKey::W:
					{
						camera->frameMovementFlags.forward = action;
					} break;
					case AB::KeyboardKey::S:
					{
						camera->frameMovementFlags.back = action;
					} break;
					case AB::KeyboardKey::A: {
						camera->frameMovementFlags.left = action;
					} break;
					case AB::KeyboardKey::D: {
						camera->frameMovementFlags.right = action;
					} break;
					INVALID_DEFAULT_CASE();
					}
				}
			};

		EventQuery w = {};
		w.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		w.condition.key_event.key = KeyboardKey::W;
		w.callback = CameraMovementCallback;

		AB::EventQuery s = {};
		s.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		s.condition.key_event.key = KeyboardKey::S;
		s.callback = CameraMovementCallback;

		AB::EventQuery a = {};
		a.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		a.condition.key_event.key = KeyboardKey::A;
		a.callback = CameraMovementCallback;

		AB::EventQuery d = {};
		d.type = EventType::EVENT_TYPE_KEY_PRESSED | EVENT_TYPE_KEY_RELEASED;
		d.condition.key_event.key = KeyboardKey::D;
		d.callback = CameraMovementCallback;

		InputSubscribeEvent(inputManager, &w);
		InputSubscribeEvent(inputManager, &s);
		InputSubscribeEvent(inputManager, &a);
		InputSubscribeEvent(inputManager, &d);

		EventQuery tabEvent = {};
		tabEvent.type = EventType::EVENT_TYPE_KEY_PRESSED;
		tabEvent.condition.key_event.key = KeyboardKey::Tab;
		tabEvent.callback = [](Event e)
			{
				inputState.mouseCaptured = !inputState.mouseCaptured;
				if (inputState.mouseCaptured)
				{
					InputSetMouseMode(inputManager, MouseMode::Captured);
				}
				else
				{
					InputSetMouseMode(inputManager, MouseMode::Cursor);
				}
			};

		InputSubscribeEvent(inputManager, &tabEvent);

		EventQuery cursorEvent = {};
		cursorEvent.type = EVENT_TYPE_MOUSE_MOVED;
		cursorEvent.callback = [](Event e)
			{
				if (inputState.mouseCaptured)
				{
					camera->pitch += e.mouse_moved_event.y;
					//g_Camera.yaw += e.mouse_moved_event.x;
					inputState.lastMouseX = e.mouse_moved_event.x;
					inputState.lastMouseY = e.mouse_moved_event.y;
					if (camera->pitch > 89.0f)
					{
						camera->pitch = 89.0f;
					}
					if (camera->pitch < -89.0f)
					{
						camera->pitch = -89.0f;
					}
				}
			};

		InputSubscribeEvent(inputManager, &cursorEvent);
	}
#endif

	
	void UpdateCamera(AnnoCamera* cam, InputManager* inputManager)
	{
		if (cam->frameMovementFlags.forward)
		{
			cam->pos = AddV3V3(MulV3F32(cam->front, 0.2), cam->pos);
		}
		if (cam->frameMovementFlags.back)
		{
			cam->pos = SubV3V3(cam->pos, MulV3F32(cam->front, 0.2));
		}
		if (cam->frameMovementFlags.left)
		{
			v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
			cam->pos = SubV3V3(cam->pos, MulV3F32(right, 0.2));
		}
		if (cam->frameMovementFlags.right)
		{
			v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
			cam->pos = AddV3V3(MulV3F32(right, 0.2), cam->pos);
		}

		if (InputMouseButtonIsDown(inputManager, MouseButton::Middle))
		{
			v2 mousePos = InputGetMouseFrameOffset(inputManager);
			cam->lastMousePos.x -= mousePos.x;// - cam->mouseUntrackedOffset;
			cam->lastMousePos.y -= mousePos.y;
			DEBUG_OVERLAY_PUSH_VAR("mouse y:", mousePos.y);
		}

		if (cam->lastMousePos.y < 95.0f)
		{
			cam->lastMousePos.y = 95.0f;
			//cam->latitude = 95.000001f;
		}
		else if (cam->lastMousePos.y > 170.0f)
		{
			cam->lastMousePos.y = 170.0f;
			//cam->latitude = 170.00001f;
		}

		
		float32 scrollSpeed = 5.0f;
		
		i32 frameScrollOffset = InputGetFrameScrollOffset(inputManager);
		cam->targetDistance -= frameScrollOffset * scrollSpeed;

		if (cam->targetDistance < 2.0f)
		{
			cam->targetDistance = 2.0f;
		} else if (cam->targetDistance > 120.0f)
		{
			cam->targetDistance = 120.0f;
		}

		cam->latitude = Lerp(cam->latitude, cam->lastMousePos.y,
							 cam->latSmoothness);

		cam->longitude = Lerp(cam->longitude, cam->lastMousePos.x,
							  cam->longSmoothness);

		cam->distance = Lerp(cam->distance, cam->targetDistance,
							 cam->distSmoothness);


		DEBUG_OVERLAY_PUSH_SLIDER("r", &cam->distance, 1.0f, 10.0f);
		DEBUG_OVERLAY_PUSH_VAR("r", cam->distance);

		DEBUG_OVERLAY_PUSH_SLIDER("target", &cam->target, 0.0f, 10.0f);
		DEBUG_OVERLAY_PUSH_VAR("target", cam->target);
		//DEBUG_OVERLAY_PUSH_SLIDER("height", &cam->latitude, 0.0f, 180.0f);
		DEBUG_OVERLAY_PUSH_VAR("height", cam->latitude);

		f32 latitude = ToRadians(cam->latitude);
		f32 longitude = ToRadians(cam->longitude);
		f32 polarAngle = PI_32 - latitude;

		f32 z = cam->target.z + cam->distance * Sin(polarAngle) * Cos(longitude);
		f32 x = cam->target.x + cam->distance * Sin(polarAngle) * Sin(longitude);
		f32 y = cam->target.y + cam->distance * Cos(polarAngle);

		cam->pos = V3(x, y, z);
		
		cam->front = Normalize(SubV3V3(cam->target, cam->pos));
		
		DEBUG_OVERLAY_PUSH_VAR("cam front", cam->front);
	}

}
