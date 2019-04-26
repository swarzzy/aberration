#define HYPERMATH_IMPL
#include <hypermath.h>

#include <Aberration.h>
#include "Application.h"
#include "render/Renderer.h"
#include "platform/InputManager.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Memory.h"
#include "AssetManager.h"
#include "platform/API/GraphicsAPI.h"
#include "platform/API/GraphicsPipeline.h"
#include "utils/ImageLoader.h"
#include "ExtendedMath.h"

using namespace AB;

struct InputState
{
	b32 mouseCaptured;
	f32 lastMouseX;
	f32 lastMouseY;
};

struct FPCamera
{
	f32 smoothness;
	f32 pitch;
	f32 yaw;
	quat rotation;
	v3 pos;
	v3 front;
	v3 initDir;

	struct
	{
		b32 forward;
		b32 back;
		b32 left;
		b32 right;
	} frameMovementFlags;
};

struct AnnoCamera
{
	f32 smoothness;
	f32 pitch;
	f32 yaw;
	quat rotation;
	v3 pos;
	v3 front;
	v3 initDir;
	v3 target;
	f32 distance;
	f32 latitude;
	v2 lastMousePos;
	f32 mouseBegPos;
	f32 mouseEndPos;
	f32 mouseUntrackedOffset;
	f32 currMouseOrigin;

	struct
	{
		b32 forward;
		b32 back;
		b32 left;
		b32 right;
	} frameMovementFlags;
};


int32 mesh;
int32 mesh2;
int32 mesh3;
int32 plane;
int32 cubeMesh;

int32 material;
int32 material1;

bool32 mouse_captured = false;
float32 last_mouse_x = 0.0f;
float32 last_mouse_y = 0.0f;

AB::DirectionalLight light = { {0, -1, 0.25}, {0.001}, {0.0}, {0.0} };
AB::PointLight plights[4];

AB::InputMgr* g_Input = nullptr;

InputState g_InputState;

float32 g_Gamma = 2.2f;
float32 g_Exposure = 1.0f;

AB::Renderer* g_Renderer;
AB::AssetManager*  asset_mgr;
int32 debugCubeMesh;

RenderGroup* g_RenderGroup;

AnnoCamera g_Camera;

void UpdateCameraFP(FPCamera* cam)
{
	if (cam->frameMovementFlags.forward)
	{
		cam->pos = Add(MulV3F32(cam->front, 0.2), cam->pos);
	}
	if (cam->frameMovementFlags.back)
	{
		cam->pos = Subtract(cam->pos, MulV3F32(cam->front, 0.2));
	}
	if (cam->frameMovementFlags.left)
	{
		v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
		cam->pos = Subtract(cam->pos, MulV3F32(right, 0.2));
	}
	if (cam->frameMovementFlags.right)
	{
		v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
		cam->pos = Add(MulV3F32(right, 0.2), cam->pos);
	}

#if 0
	cam_front.x = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Cos(hpm::ToRadians(yaw));
	cam_front.y = hpm::Sin(hpm::ToRadians(pitch));
	cam_front.z = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Sin(hpm::ToRadians(yaw));
	cam_front = hpm::Normalize(cam_front);
#endif
	
	quat qPitch = QuatFromAxisAngle(V3(1, 0, 0), ToRadians(cam->pitch));
    //qPitch = Normalize(qPitch);
	quat qYaw = QuatFromAxisAngle(V3(0, 1, 0), ToRadians(cam->yaw));
	//qYaw = Normalize(qYaw);
	//quat camRot = QuatFromEuler(ToRadians(yawOffset), ToRadians(pitchOffset), 0);//MulQQ(qPitch, qYaw);
	quat frameRotation = MulQQ(qPitch, qYaw);
	
	DEBUG_OVERLAY_PUSH_SLIDER("cam smooth", &cam->smoothness, 0.0f, 1.0f);
	DEBUG_OVERLAY_PUSH_VAR("cam smooth", cam->smoothness);
	
	cam->rotation = LerpQuat(cam->rotation, frameRotation, cam->smoothness);
	m3x3 rotMat = QuatToM3x3(cam->rotation);
	cam->front = MulM3V3(rotMat, cam->initDir);
	//cam_front = Rotate(camRotationQ, V3(0, 0, 1));
	cam->front = Normalize(cam->front);
	DEBUG_OVERLAY_PUSH_VAR("cam front", cam->front);
}

void UpdateCamera(AnnoCamera* cam)
{
	if (cam->frameMovementFlags.forward)
	{
		cam->pos = Add(MulV3F32(cam->front, 0.2), cam->pos);
	}
	if (cam->frameMovementFlags.back)
	{
		cam->pos = Subtract(cam->pos, MulV3F32(cam->front, 0.2));
	}
	if (cam->frameMovementFlags.left)
	{
		v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
		cam->pos = Subtract(cam->pos, MulV3F32(right, 0.2));
	}
	if (cam->frameMovementFlags.right)
	{
		v3 right = Normalize(Cross(cam->front, { 0, 1, 0 }));
		cam->pos = Add(MulV3F32(right, 0.2), cam->pos);
	}

#if 0
	cam_front.x = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Cos(hpm::ToRadians(yaw));
	cam_front.y = hpm::Sin(hpm::ToRadians(pitch));
	cam_front.z = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Sin(hpm::ToRadians(yaw));
	cam_front = hpm::Normalize(cam_front);
#endif


	if (InputMouseButtonIsPressed(g_Input, MouseButton::Middle))
	{
		cam->lastMousePos.x = cam->yaw;
		cam->mouseBegPos = InputGetMousePosition(g_Input).x;
		cam->mouseUntrackedOffset = cam->mouseBegPos - cam->mouseEndPos;
	}
	else if (InputMouseButtonIsDown(g_Input, MouseButton::Middle))
	{
		cam->lastMousePos.x = InputGetMousePosition(g_Input).x - cam->mouseUntrackedOffset;
	}

	if (InputMouseButtonIsReleased(g_Input, MouseButton::Middle))
	{
		cam->mouseEndPos = InputGetMousePosition(g_Input).x;
	}

	cam->yaw = Lerp(cam->yaw, cam->lastMousePos.x, 0.1);


	DEBUG_OVERLAY_PUSH_SLIDER("r", &cam->distance, 1.0f, 10.0f);
	DEBUG_OVERLAY_PUSH_VAR("r", cam->distance);

	DEBUG_OVERLAY_PUSH_SLIDER("target", &cam->target, 0.0f, 10.0f);
	DEBUG_OVERLAY_PUSH_VAR("target", cam->target);
	DEBUG_OVERLAY_PUSH_SLIDER("height", &cam->latitude, 0.0f, 90.0f);
	DEBUG_OVERLAY_PUSH_VAR("height", cam->latitude);

	f32 camPosX = cam->distance * Cos(ToRadians(cam->yaw));
	f32 camPosZ = cam->distance * Sin(ToRadians(cam->yaw));

	f32 camPosY = cam->latitude * Sin(ToRadians(cam->distance));

	v3 dir = Subtract(cam->target, V3(camPosX, camPosY, camPosZ));

	cam->pos = V3(camPosX, camPosY, camPosZ);
	cam->front = dir;
		
	
	//quat qPitch = QuatFromAxisAngle(V3(1, 0, 0), ToRadians(cam->pitch));
    //qPitch = Normalize(qPitch);
	//quat qYaw = QuatFromAxisAngle(V3(0, 1, 0), ToRadians(cam->yaw));
	//qYaw = Normalize(qYaw);
	//quat camRot = QuatFromEuler(ToRadians(yawOffset), ToRadians(pitchOffset), 0);//MulQQ(qPitch, qYaw);
	//quat frameRotation = MulQQ(qPitch, qYaw);
	
	//DEBUG_OVERLAY_PUSH_SLIDER("cam smooth", &cam->smoothness, 0.0f, 1.0f);
	//DEBUG_OVERLAY_PUSH_VAR("cam smooth", cam->smoothness);
	
	//cam->rotation = LerpQuat(cam->rotation, frameRotation, cam->smoothness);
	//m3x3 rotMat = QuatToM3x3(cam->rotation);
	//cam->front = MulM3V3(rotMat, cam->initDir);
	//cam_front = Rotate(camRotationQ, V3(0, 0, 1));
	//cam->front = Normalize(cam->front);
	DEBUG_OVERLAY_PUSH_VAR("cam front", cam->front);
}


void SubscribeKeyboardEvents()
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
					g_Camera.frameMovementFlags.forward = action;
				} break;
				case AB::KeyboardKey::S:
				{
					g_Camera.frameMovementFlags.back = action;
				} break;
				case AB::KeyboardKey::A: {
					g_Camera.frameMovementFlags.left = action;
				} break;
				case AB::KeyboardKey::D: {
					g_Camera.frameMovementFlags.right = action;
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

	InputSubscribeEvent(g_Input, &w);
	InputSubscribeEvent(g_Input, &s);
	InputSubscribeEvent(g_Input, &a);
	InputSubscribeEvent(g_Input, &d);

	EventQuery tabEvent = {};
	tabEvent.type = EventType::EVENT_TYPE_KEY_PRESSED;
	tabEvent.condition.key_event.key = KeyboardKey::Tab;
	tabEvent.callback = [](Event e)
	{
		g_InputState.mouseCaptured = !g_InputState.mouseCaptured;
		if (g_InputState.mouseCaptured)
		{
			InputSetMouseMode(g_Input, MouseMode::Captured);
		}
		else
		{
			InputSetMouseMode(g_Input, MouseMode::Cursor);
		}
	};

	InputSubscribeEvent(g_Input, &tabEvent);

	EventQuery cursorEvent = {};
	cursorEvent.type = EVENT_TYPE_MOUSE_MOVED;
	cursorEvent.callback = [](Event e)
	{
		if (g_InputState.mouseCaptured)
		{
			g_Camera.pitch += e.mouse_moved_event.y;
			//g_Camera.yaw += e.mouse_moved_event.x;
			g_InputState.lastMouseX = e.mouse_moved_event.x;
			g_InputState.lastMouseY = e.mouse_moved_event.y;
			if (g_Camera.pitch > 89.0f)
			{
				g_Camera.pitch = 89.0f;
			}
			if (g_Camera.pitch < -89.0f)
			{
				g_Camera.pitch = -89.0f;
			}
		}
	};

	InputSubscribeEvent(g_Input, &cursorEvent);
}


void Init() {
	g_Camera = {};
	g_Camera.smoothness = 0.5f;
	g_Camera.pos = V3(0.0f, 0.0f, -1.0f);
	g_Camera.initDir = V3(0.0f, 0.0f, 1.0f);
	
	AB::RendererConfig config;
	config.numSamples = 4;
	config.renderResolutionW = 1280;
	config.renderResolutionH = 720;

	API::Pipeline* pipeline = (API::Pipeline*)SysAlloc(sizeof(API::Pipeline));
	API::InitPipeline(pipeline);
	
	g_Renderer = AB::RendererInit(config, pipeline);
	g_RenderGroup = RenderGroupAllocate(AB::GetMemory(), MEGABYTES(1), 256, 32);
	g_RenderGroup->projectionMatrix = PerspectiveRH(45.0f,
													16.0f / 9.0f,
													0.1f,
													100.0f);
	g_Input = AB::InputInitialize();
	asset_mgr = AB::AssetInitialize();
	mesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/mansion/mansion.aab");
	mesh2 = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/barrel2.aab");
	mesh3 = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/alignedBarrel.aab");
	plane = AB::AssetCreateMeshAAB(asset_mgr, "../assets/Plane.aab");
	cubeMesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/grass.aab");
	debugCubeMesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/Cube.aab");

	AB::Image px = AB::LoadBMP("../assets/cubemap/posx.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image nx = AB::LoadBMP("../assets/cubemap/negx.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image py = AB::LoadBMP("../assets/cubemap/posy.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image ny = AB::LoadBMP("../assets/cubemap/negy.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image pz = AB::LoadBMP("../assets/cubemap/posz.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image nz = AB::LoadBMP("../assets/cubemap/negz.bmp", AB::API::TEX_COLOR_SPACE_SRGB);

	AB::API::TextureParameters p = {AB::API::TEX_FILTER_LINEAR, AB::API::TEX_WRAP_CLAMP_TO_EDGE};
	uint32 cubemap = AB::API::CreateCubemap(p, &px, &nx, &py, &ny, &pz, &nz);
	AB::RendererSetSkybox(g_Renderer, cubemap);

	SubscribeKeyboardEvents();

	light.ambient = { 0.002, 0.002, 0.002 };
	light.diffuse = { 0.3 , 0.3 , 0.3 };
	light.specular = { 0.5, 0.5, 0.5 };
}

void Update() {
	
}

Vector3 lpos = {0, 0, 49.5};
float32 redLightInt = 0.2f;
float32 prevScale = 1.0f;
float32 currScale = 1.0f;
bool32 multisamplingPrev = false;
bool32 multisampling = false;

float32 g_Time = 0.0f;

void Render() {
	g_Time += 0.1;
	AB::InputBeginFrame(g_Input);

	UpdateCamera(&g_Camera);
	RenderGroupSetCamera(g_RenderGroup, g_Camera.front, g_Camera.pos);

	auto tr = hpm::Translation({ 0, 0, 0 });
	auto cubeTransform = Translation({0, 0, 0});
	cubeTransform = Scale(cubeTransform, {1, 1, 1});
	m4x4 grass2Transform = Translation({1.0f, 0.0f, 0.0f});
	//AB::RendererBeginFrame(g_Renderer);
	RenderGroupResetQueue(g_RenderGroup);

	//to testTransform = Translation({g_Time / 2, 0, 0});
	//testTransform = Scale(testTransform, {2,2,2});
	auto testTransform = Identity4();//Rotation(g_Time, {0.2, 0.3, 0.7});
	Vector3 trans = GetPosition(&testTransform);

	auto boxTransform = Translation(trans);

	AB::BBoxAligned aabb = AB::AssetGetMeshData(asset_mgr, mesh2)->aabb;
	
	
	Matrix4 aabbTr1 = Identity4();
	Matrix4 aabbTr2 = Identity4();
	Matrix4 aabbTr3 = Identity4();
	Matrix4 aabbTr4 = Identity4();
	Matrix4 aabbTr5 = Identity4();
	Matrix4 aabbTr6 = Identity4();
	Matrix4 aabbTr7 = Identity4();
	Matrix4 aabbTr8 = Identity4();

	aabbTr1 = Rotate(aabbTr1, g_Time, {0.2, 0.3, 0.7});
	aabbTr2 = Rotate(aabbTr2, g_Time, {0.2, 0.3, 0.7});
	aabbTr3 = Rotate(aabbTr3, g_Time, {0.2, 0.3, 0.7});
	aabbTr4 = Rotate(aabbTr4, g_Time, {0.2, 0.3, 0.7});
	aabbTr5 = Rotate(aabbTr5, g_Time, {0.2, 0.3, 0.7});
	aabbTr6 = Rotate(aabbTr6, g_Time, {0.2, 0.3, 0.7});
	aabbTr7 = Rotate(aabbTr7, g_Time, {0.2, 0.3, 0.7});
	aabbTr8 = Rotate(aabbTr8, g_Time, {0.2, 0.3, 0.7});

	aabbTr1 = Translate(aabbTr1, {aabb.min.x, aabb.min.y, aabb.min.z});
	aabbTr2 = Translate(aabbTr2, {aabb.min.x, aabb.min.y, aabb.max.z});
	aabbTr3 = Translate(aabbTr3, {aabb.max.x, aabb.min.y, aabb.min.z});
	aabbTr4 = Translate(aabbTr4, {aabb.max.x, aabb.min.y, aabb.max.z});
	aabbTr5 = Translate(aabbTr5, {aabb.min.x, aabb.max.y, aabb.min.z});
	aabbTr6 = Translate(aabbTr6, {aabb.min.x, aabb.max.y, aabb.max.z});
	aabbTr7 = Translate(aabbTr7, {aabb.max.x, aabb.max.y, aabb.min.z});
	aabbTr8 = Translate(aabbTr8, {aabb.max.x, aabb.max.y, aabb.max.z});
	
    aabbTr1 = Scale(aabbTr1, {0.2, 0.2, 0.2});
	aabbTr2 = Scale(aabbTr2, {0.2, 0.2, 0.2});
	aabbTr3 = Scale(aabbTr3, {0.2, 0.2, 0.2});
	aabbTr4 = Scale(aabbTr4, {0.2, 0.2, 0.2});
	aabbTr5 = Scale(aabbTr5, {0.2, 0.2, 0.2});
	aabbTr6 = Scale(aabbTr6, {0.2, 0.2, 0.2});
	aabbTr7 = Scale(aabbTr7, {0.2, 0.2, 0.2});
	aabbTr8 = Scale(aabbTr8, {0.2, 0.2, 0.2});
	
	AB::RenderCommandDrawMesh planeCommand = {};
	planeCommand.meshHandle = plane;
	planeCommand.transform.worldMatrix = tr;
	planeCommand.blendMode = AB::BLEND_MODE_OPAQUE;

	m4x4 quatTransform = Identity4();
	v3 offset = V3(1.0f, 0.0f, 0.0f);
	quat rot = QuatFromAxisAngle(Normalize(V3(0, 1, 0)), g_Time * 0.2f);
	quat rot1 = QuatFromAxisAngle(Normalize(V3(1, 0, 0)), g_Time * 0.1f);
	rot = MulQQ(rot, rot1);
	//offset = Rotate(rot, offset);
	m3x3 rMtx = QuatToM3x3(rot);
	quatTransform = MulM4M4(quatTransform, M4x4(rMtx));
   	//quatTransform = MulM4V4(quatTransform, M4x4(rMtx));

	DEBUG_OVERLAY_PUSH_VAR("ct", g_Camera.target);

	//quatTransform = Translation(offset);
	quatTransform = Identity4();
	//m4x4 camTarget = Scaling(V3(0.01));
	m4x4 camTarget = Translation(g_Camera.target);
	//camTarget = Translate(camTarget, g_Camera.target);
	camTarget = Scale(camTarget, V3(0.01));


	AB::RenderCommandDrawMesh meshCommand = {};
	meshCommand.meshHandle = mesh;
	meshCommand.transform.worldMatrix = camTarget;
	meshCommand.blendMode = AB::BLEND_MODE_OPAQUE;
	
	AB::RenderCommandDrawMesh mesh1Command = {};
	mesh1Command.meshHandle = mesh2;
	mesh1Command.transform.worldMatrix = testTransform;
	mesh1Command.blendMode = AB::BLEND_MODE_OPAQUE;

	AB::RenderCommandDrawMesh mesh2Command = {};
	mesh2Command.meshHandle = mesh3;
	mesh2Command.transform.worldMatrix = testTransform;
	mesh2Command.blendMode = AB::BLEND_MODE_OPAQUE;

	AB::RenderCommandDrawMesh cubeMeshCommand = {};
	cubeMeshCommand.meshHandle = cubeMesh;
	cubeMeshCommand.transform.worldMatrix = cubeTransform;
	cubeMeshCommand.blendMode = AB::BLEND_MODE_TRANSPARENT;
	cubeMeshCommand.sortCriteria = AB::RENDER_SORT_CRITERIA_NEAREST_VERTEX;
	
	AB::RenderCommandDrawMesh grass2MeshCommand = {};
	grass2MeshCommand.meshHandle = cubeMesh;
	grass2MeshCommand.transform.worldMatrix = grass2Transform;
	grass2MeshCommand.blendMode = AB::BLEND_MODE_TRANSPARENT;
	grass2MeshCommand.sortCriteria = AB::RENDER_SORT_CRITERIA_NEAREST_VERTEX;

	AB::RenderCommandDrawMeshWireframe debugBoxCommand = {};
	debugBoxCommand.meshHandle = debugCubeMesh;
	debugBoxCommand.transform.worldMatrix = boxTransform;
	debugBoxCommand.blendMode = AB::BLEND_MODE_OPAQUE;
	debugBoxCommand.lineWidth = 2.0f;

	AB::RenderCommandDrawMesh aabb1 = {};
	aabb1.meshHandle = debugCubeMesh;
	aabb1.transform.worldMatrix = aabbTr1;
	aabb1.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb2 = {};
	aabb2.meshHandle = debugCubeMesh;
	aabb2.transform.worldMatrix = aabbTr2;
	aabb2.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb3 = {};
	aabb3.meshHandle = debugCubeMesh;
	aabb3.transform.worldMatrix = aabbTr3;
	aabb3.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb4 = {};
	aabb4.meshHandle = debugCubeMesh;
	aabb4.transform.worldMatrix = aabbTr4;
	aabb4.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb5 = {};
	aabb5.meshHandle = debugCubeMesh;
	aabb5.transform.worldMatrix = aabbTr5;
	aabb5.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb6 = {};
	aabb6.meshHandle = debugCubeMesh;
	aabb6.transform.worldMatrix = aabbTr6;
	aabb6.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb7 = {};
	aabb7.meshHandle = debugCubeMesh;
	aabb7.transform.worldMatrix = aabbTr7;
	aabb7.blendMode = AB::BLEND_MODE_TRANSPARENT;

	AB::RenderCommandDrawMesh aabb8 = {};
	aabb8.meshHandle = debugCubeMesh;
	aabb8.transform.worldMatrix = aabbTr8;
	aabb8.blendMode = AB::BLEND_MODE_TRANSPARENT;

	f32 ka = light.ambient.r;
	f32 kd = light.diffuse.r;
	
	DEBUG_OVERLAY_PUSH_SLIDER("Ka", &ka, 0.0f, 1.0f);
	DEBUG_OVERLAY_PUSH_VAR("Ka", ka);

	DEBUG_OVERLAY_PUSH_SLIDER("Kd", &kd, 0.0f, 10.0f);
	DEBUG_OVERLAY_PUSH_VAR("Kd", kd);

	light.ambient = V3(ka);
	light.diffuse = V3(kd);

	RenderCommandSetDirLight dirLightCommand = {};
	dirLightCommand.light = light;

	f32 p1ka = plights[0].ambient.r;
	f32 p1kd = plights[0].diffuse.r;
	f32 p1ks = plights[0].specular.r;

#if 0
	
	DEBUG_OVERLAY_PUSH_SLIDER("Ka", &p1ka, 0.0f, 1.0f);
	DEBUG_OVERLAY_PUSH_VAR("Ka", p1ka);

	DEBUG_OVERLAY_PUSH_SLIDER("Kd", &p1kd, 0.0f, 10.0f);
	DEBUG_OVERLAY_PUSH_VAR("Kd", p1kd);

	DEBUG_OVERLAY_PUSH_SLIDER("Ks", &p1ks, 0.0f, 30.0f);
	DEBUG_OVERLAY_PUSH_VAR("Ks", p1ks);

	DEBUG_OVERLAY_PUSH_SLIDER("l1 pos", &plights[0].position, -10, 10);
	DEBUG_OVERLAY_PUSH_VAR("l1 pos", plights[0].position);
#endif
	
	plights[0].ambient = V3(p1ka, p1ka, p1ka);
	plights[0].diffuse = V3(p1kd, p1kd, p1kd);
	plights[0].specular = V3(p1ks, p1ks, p1ks);
	plights[0].linear = 0.22f;
	plights[0].quadratic = 0.20f;


	RenderCommandSetPointLight pl1 = {};
	pl1.light = plights[0];

	f32 p2ka = plights[1].ambient.r;
	f32 p2kd = plights[1].diffuse.r;
	f32 p2ks = plights[1].specular.r;
	
#if 0	
	DEBUG_OVERLAY_PUSH_SLIDER("Ka", &p2ka, 0.0f, 1.0f);
	DEBUG_OVERLAY_PUSH_VAR("Ka", p2ka);

	DEBUG_OVERLAY_PUSH_SLIDER("Kd", &p2kd, 0.0f, 10.0f);
	DEBUG_OVERLAY_PUSH_VAR("Kd", p2kd);

	DEBUG_OVERLAY_PUSH_SLIDER("Ks", &p2ks, 0.0f, 30.0f);
	DEBUG_OVERLAY_PUSH_VAR("Ks", p2ks);

	DEBUG_OVERLAY_PUSH_SLIDER("l1 pos", &plights[1].position, -10, 10);
	DEBUG_OVERLAY_PUSH_VAR("l1 pos", plights[1].position);
#endif
	
	plights[1].ambient = V3(p2ka, p2ka, 0);
	plights[1].diffuse = V3(p2kd, p2kd, 0);
	plights[1].specular = V3(p2ks, p2ks, 0);
	plights[1].linear = 0.22f;
	plights[1].quadratic = 0.20f;


	RenderCommandSetPointLight pl2 = {};
	pl2.light = plights[1];




	AB::RenderGroupPushCommand(g_RenderGroup,
							   AB::RENDER_COMMAND_DRAW_MESH,
							   (void*)(&cubeMeshCommand));

	RenderGroupPushCommand(g_RenderGroup,
						   AB::RENDER_COMMAND_DRAW_MESH,
						   (void*)(&meshCommand));

	AB::RenderGroupPushCommand(g_RenderGroup,
							   AB::RENDER_COMMAND_DRAW_MESH,
							   (void*)(&planeCommand));



	AB::RenderGroupPushCommand(g_RenderGroup,
							   AB::RENDER_COMMAND_DRAW_MESH,
							   (void*)(&grass2MeshCommand));

	RenderGroupPushCommand(g_RenderGroup,
						   RENDER_COMMAND_SET_DIR_LIGHT,
						   (void*)(&dirLightCommand));

	RenderGroupPushCommand(g_RenderGroup,
						   RENDER_COMMAND_SET_POINT_LIGHT,
						   (void*)(&pl1));

	RenderGroupPushCommand(g_RenderGroup,
						   RENDER_COMMAND_SET_POINT_LIGHT,
						   (void*)(&pl2));



#if 0


	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&mesh1Command));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&mesh2Command));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH_WIREFRAME,
							(void*)(&debugBoxCommand));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb1));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb2));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb3));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb4));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb5));


	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb6));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb7));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&aabb8));

	AB::RendererEndFrame(g_Renderer);
	
#endif

	DEBUG_OVERLAY_PUSH_SLIDER("gamma", &g_Gamma, -10, 10);
	DEBUG_OVERLAY_PUSH_VAR("gamma ", g_Gamma);
	DEBUG_OVERLAY_PUSH_SLIDER("Exp", &g_Exposure, -2, 5);
	DEBUG_OVERLAY_PUSH_VAR("Exp", g_Exposure);
	//DEBUG_OVERLAY_PUSH_SLIDER("I", &redLightInt, 0, 100);
	//DEBUG_OVERLAY_PUSH_VAR("I", redLightInt);
	DEBUG_OVERLAY_PUSH_SLIDER("Resolution:", &currScale, 0.1f, 4.0f);
	DEBUG_OVERLAY_PUSH_VAR("Resolution:", currScale);
	DEBUG_OVERLAY_PUSH_TOGGLE("MSAA 8x", &multisampling);

	AB::InputEndFrame(g_Input);

	if (Abs(currScale - prevScale) > 0.01) {
		auto config = AB::RendererGetConfig(g_Renderer);
		config.renderResolutionW = (uint16)(1280 * currScale);
		config.renderResolutionH = (uint16)(720 * currScale);
		AB::RendererApplyConfig(g_Renderer, &config);
		prevScale = currScale;
	}

	if (multisampling != multisamplingPrev) {
		auto config = AB::RendererGetConfig(g_Renderer);
		config.numSamples = multisampling ? 8 : 0;
		AB::RendererApplyConfig(g_Renderer, &config);
		multisamplingPrev = multisampling;		
	}
	
	plights[0].linear = 0.14;
	plights[0].quadratic = 0.07;

	g_Renderer->cc.gamma = g_Gamma;
	g_Renderer->cc.exposure = g_Exposure;

	//DEBUG_OVERLAY_PUSH_SLIDER("l", &lpos, 0, 55);

	//AB::RendererSetDirectionalLight(g_Renderer, &light);
	//plights[0] = {lpos, {0.1, 0.1, 0.1}, {10, 10, 10}, {}, 0.22, 0.20};
	//plights[0] = {{0, 0, 45.5}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.2}, {0.0,0.0, 0.0}, 0.22, 0.20};
	//plights[1] = {{-1.4, -1.9, 9.0}, {}, {redLightInt, 0, 0}, {}, 0, 1};
	//plights[2] = {{0, -1.8, 4.0}, {}, {0, 0, 0.2}, {}, 0, 1};
	//plights[3] = {{0.8, -1.7, 6.0}, {}, {0.0, 0.1, 0.0}, {}, 0, 1};
	//AB::RendererSubmitPointLight(g_Renderer, &plights[0]);
	//AB::RendererSubmitPointLight(g_Renderer, &plights[1]);
	//AB::RendererSubmitPointLight(g_Renderer, &plights[2]);
	//AB::RendererSubmitPointLight(g_Renderer, &plights[3]);

	AB::RendererRender(g_Renderer, g_RenderGroup);
}

int EntryPoint() {
	AB::CreateMemoryContext();
	auto* app = AB::AppCreate();
	AB::AppSetInitCallback(app, Init);
	AB::AppSetUpdateCallback(app, Update);
	AB::AppSetRenderCallback(app, Render);
	AB::AppRun(app);
	return 0;
}

AB_DECLARE_ENTRY_POINT(EntryPoint)
