#define HYPERMATH_IMPL
#include <hypermath.h>

#include <Aberration.h>
#include "Application.h"
#include "renderer/Renderer3D.h"
#include "platform/InputManager.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Memory.h"
#include "AssetManager.h"
#include "platform/API/GraphicsAPI.h"
#include "utils/ImageLoader.h"
#include "ExtendedMath.h"

int32 mesh;
int32 mesh2;
int32 mesh3;
int32 plane;
int32 cubeMesh;

int32 material;
int32 material1;

float32 pitch = 0;
float32 yaw = 0;
hpm::Vector3 cam_pos = {0, 0, 0};
hpm::Vector3 cam_front = { 0, 0, -1 };

bool32 mouse_captured = false;
float32 last_mouse_x = 0.0f;
float32 last_mouse_y = 0.0f;

AB::DirectionalLight light = { {0, -1, 0.25}, {0.001}, {0.0}, {0.0} };
AB::PointLight plights[4];

AB::InputMgr* g_Input = nullptr;

bool32 g_cam_w = 0;
bool32 g_cam_s = 0;
bool32 g_cam_a = 0;
bool32 g_cam_d = 0;

float32 g_Gamma = 2.2f;
float32 g_Exposure = 1.0f;

void MouseCallback(AB::Event e) {
	if (mouse_captured) {
		pitch += e.mouse_moved_event.y;
		yaw += e.mouse_moved_event.x;
		last_mouse_x = e.mouse_moved_event.x;
		last_mouse_y = e.mouse_moved_event.y;
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < -360.0f)
			yaw -= -360.0f;
	}
}

auto move_callback = [](AB::Event e) {
	int32 action = -1;
	if (e.type == AB::EVENT_TYPE_KEY_PRESSED) {
		action = 1;
	}
	else if (e.type == AB::EVENT_TYPE_KEY_RELEASED) {
		action = 0;
	}
	if (action != -1) {
		switch (e.key_event.key) {
		case AB::KeyboardKey::W: {
			g_cam_w = action;
		} break;
		case AB::KeyboardKey::S: {
			g_cam_s = action;
		} break;
		case AB::KeyboardKey::A: {
			g_cam_a = action;
		} break;
		case AB::KeyboardKey::D: {
			g_cam_d = action;
		} break;
		default: {
		} break;
		}
	}
};

int32 w_h;
int32 a_h;
int32 s_h;
int32 d_h;

void Subscribe() {
	AB::EventQuery w_q = {};
	w_q.type = AB::EventType::EVENT_TYPE_KEY_PRESSED | AB::EVENT_TYPE_KEY_RELEASED;
	//w_q.pass_through = true;
	w_q.condition.key_event.key = AB::KeyboardKey::W;
	w_q.callback = move_callback;

	AB::EventQuery s_q = {};
	s_q.type = AB::EventType::EVENT_TYPE_KEY_PRESSED | AB::EVENT_TYPE_KEY_RELEASED;
	//s_q.pass_through = true;
	s_q.condition.key_event.key = AB::KeyboardKey::S;
	s_q.callback = move_callback;

	AB::EventQuery a_q = {};
	a_q.type = AB::EventType::EVENT_TYPE_KEY_PRESSED | AB::EVENT_TYPE_KEY_RELEASED;
	//a_q.pass_through = true;
	a_q.condition.key_event.key = AB::KeyboardKey::A;
	a_q.callback = move_callback;

	AB::EventQuery d_q = {};
	d_q.type = AB::EventType::EVENT_TYPE_KEY_PRESSED | AB::EVENT_TYPE_KEY_RELEASED;
	//d_q.pass_through = true;
	d_q.condition.key_event.key = AB::KeyboardKey::D;
	d_q.callback = move_callback;

	w_h = AB::InputSubscribeEvent(g_Input, &w_q);
	s_h = AB::InputSubscribeEvent(g_Input, &s_q);
	a_h = AB::InputSubscribeEvent(g_Input, &a_q);
	d_h = AB::InputSubscribeEvent(g_Input, &d_q);
}

AB::Renderer* g_Renderer;
AB::AssetManager*  asset_mgr;
int32 debugCubeMesh;

void Init() {
	GLint v;
	AB::PrintString("Mtx4: %u64\n", alignof(Matrix4));
	AB::PrintString("RenderCommandDrawMesh aligment: %u64", sizeof(AB::RenderCommandDrawMesh));
	glGetIntegerv(GL_MAX_SAMPLES, &v);
	AB::PrintString("Max Samples: %u32\n", v);
	AB::RendererConfig config;
	config.numSamples = 4;
	config.renderResolutionW = 1280;
	config.renderResolutionH = 720;
	
	g_Renderer = AB::RendererInit(config);
	AB::RendererAllocateBuffers(AB::GetMemory(), g_Renderer, MEGABYTES(1), 256);
	g_Input = AB::InputInitialize();
	asset_mgr = AB::AssetInitialize();
	mesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/barrel1.aab");
	mesh2 = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/barrel2.aab");
	mesh3 = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/alignedBarrel.aab");
	plane = AB::AssetCreateMeshAAB(asset_mgr, "../assets/Plane.aab");
	cubeMesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/grass.aab");
	debugCubeMesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/Cube.aab");
	Subscribe();

	AB::Image px = AB::LoadBMP("../assets/cubemap/posx.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image nx = AB::LoadBMP("../assets/cubemap/negx.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image py = AB::LoadBMP("../assets/cubemap/posy.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image ny = AB::LoadBMP("../assets/cubemap/negy.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image pz = AB::LoadBMP("../assets/cubemap/posz.bmp", AB::API::TEX_COLOR_SPACE_SRGB);
	AB::Image nz = AB::LoadBMP("../assets/cubemap/negz.bmp", AB::API::TEX_COLOR_SPACE_SRGB);

	AB::API::TextureParameters p = {AB::API::TEX_FILTER_LINEAR, AB::API::TEX_WRAP_CLAMP_TO_EDGE};
	uint32 cubemap = AB::API::CreateCubemap(p, &px, &nx, &py, &ny, &pz, &nz);
	AB::RendererSetSkybox(g_Renderer, cubemap);
	
	AB::EventQuery tab_q = {};
	tab_q.type = AB::EventType::EVENT_TYPE_KEY_PRESSED;
	//tab_q.pass_through = true;
	tab_q.condition.key_event.key = AB::KeyboardKey::Tab;
	tab_q.callback = [](AB::Event e) {
		mouse_captured = !mouse_captured;
		if (mouse_captured) {
			AB::InputSetMouseMode(g_Input, AB::MouseMode::Captured);
		}
		else {
			AB::InputSetMouseMode(g_Input, AB::MouseMode::Cursor);
		}
	};

	AB::InputSubscribeEvent(g_Input, &tab_q);

	m4x4 m = Translation({1, 2, 3});
	v4 vec = MulM4V4(m, {9, 8, 7, 1});
	AB::PrintString("Vector: %f32, %f32, %f32\n", vec.x, vec.y, vec.z);

	v3 sd = V3(1, 2, 3);
	v2 fee = {};
	v3 f4 = V3(fee, 3.0f);
	v4 vb = V4(1.0f, 2.0f, 3.0f, 4.0f);
	v4 fff = V4(V3(V2(1.0f, 2.0f), 3.0f), 4.0f);
	
	AB::EventQuery m_move_q = {};
	m_move_q.type = AB::EVENT_TYPE_MOUSE_MOVED;
	//m_move_q.pass_through = true;
	m_move_q.callback = [](AB::Event e) {
		if (mouse_captured) {
			pitch += e.mouse_moved_event.y;
			yaw += e.mouse_moved_event.x;
			last_mouse_x = e.mouse_moved_event.x;
			last_mouse_y = e.mouse_moved_event.y;
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			if (yaw > 360.0f)
				yaw -= 360.0f;
			if (yaw < -360.0f)
				yaw -= -360.0f;
		}
	};

	AB::InputSubscribeEvent(g_Input, &m_move_q);

	AB::EventQuery click = {};
	click.type = AB::EventType::EVENT_TYPE_MOUSE_BTN_PRESSED| AB::EVENT_TYPE_MOUSE_BTN_RELEASED;
	click.condition.mouse_button_event.button = AB::MouseButton::Left;
	click.callback = [](AB::Event e) {
		if (e.type == AB::EVENT_TYPE_MOUSE_BTN_PRESSED) {
			AB::PrintString("pressed\n");
		} else {
			AB::PrintString("released\n");
		}
	};

	AB::InputSubscribeEvent(g_Input, &click);		
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
	
	if (g_cam_w) {
		cam_pos = hpm::Add(hpm::MulV3F32(cam_front, 0.2), cam_pos);
	}
	if (g_cam_s) {
		cam_pos = hpm::Subtract(cam_pos, hpm::MulV3F32(cam_front, 0.2));
	}

	if (g_cam_a) {
		hpm::Vector3 right = hpm::Normalize(hpm::Cross(cam_front, { 0, 1, 0 }));
		cam_pos = hpm::Subtract(cam_pos, hpm::MulV3F32(right, 0.2));
	}
	if (g_cam_d) {
		hpm::Vector3 right = hpm::Normalize(hpm::Cross(cam_front, { 0, 1, 0 }));
		cam_pos = hpm::Add(hpm::MulV3F32(right, 0.2), cam_pos);
	}

	cam_front.x = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Cos(hpm::ToRadians(yaw));
	cam_front.y = hpm::Sin(hpm::ToRadians(pitch));
	cam_front.z = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Sin(hpm::ToRadians(yaw));
	cam_front = hpm::Normalize(cam_front);
	AB::RendererSetCamera(g_Renderer, cam_front, cam_pos);

	auto tr = hpm::Translation({ 0, 0, 0 });
	auto cubeTransform = Translation({0, 0, 0});
	cubeTransform = Scale(cubeTransform, {1, 1, 1});
	m4x4 grass2Transform = Translation({1.0f, 0.0f, 0.0f});
	AB::RendererBeginFrame(g_Renderer);

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

	AB::RenderCommandDrawMesh meshCommand = {};
	meshCommand.meshHandle = mesh;
	meshCommand.transform.worldMatrix = tr;
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

	AB::PrintString("---------------\n");

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&cubeMeshCommand));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&planeCommand));

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&grass2MeshCommand));


#if 0

	AB::RendererPushCommand(g_Renderer,
							AB::RENDER_COMMAND_DRAW_MESH,
							(void*)(&meshCommand));

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

	auto* settings = AB::RendererGetFlySettings(g_Renderer);
	settings->gamma = g_Gamma;
	settings->exposure = g_Exposure;

	//DEBUG_OVERLAY_PUSH_SLIDER("l", &lpos, 0, 55);

	light.ambient = { 0.2, 0.2, 0.2 };
	light.diffuse = { 3.0 , 3.0 , 3.0 };
	light.specular = { 10.0, 10.0, 10.0 };
	AB::RendererSetDirectionalLight(g_Renderer, &light);
	//plights[0] = {lpos, {0.1, 0.1, 0.1}, {10, 10, 10}, {}, 0.22, 0.20};
	//plights[0] = {{0, 0, 45.5}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.2}, {0.0,0.0, 0.0}, 0.22, 0.20};
	//plights[1] = {{-1.4, -1.9, 9.0}, {}, {redLightInt, 0, 0}, {}, 0, 1};
	//plights[2] = {{0, -1.8, 4.0}, {}, {0, 0, 0.2}, {}, 0, 1};
	//plights[3] = {{0.8, -1.7, 6.0}, {}, {0.0, 0.1, 0.0}, {}, 0, 1};
	//AB::RendererSubmitPointLight(g_Renderer, &plights[0]);
	//AB::RendererSubmitPointLight(g_Renderer, &plights[1]);
	//AB::RendererSubmitPointLight(g_Renderer, &plights[2]);
	//AB::RendererSubmitPointLight(g_Renderer, &plights[3]);

	AB::RendererRender(g_Renderer);
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
