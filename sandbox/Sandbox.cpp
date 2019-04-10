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

void Init() {
	GLint v;
	glGetIntegerv(GL_MAX_SAMPLES, &v);
	AB::PrintString("Max Samples: %u32\n", v);
	AB::RendererConfig config;
	config.numSamples = 4;
	config.renderResolutionW = 1280;
	config.renderResolutionH = 720;
	
	g_Renderer = AB::RendererInit(config);
	g_Input = AB::InputInitialize();
	auto asset_mgr = AB::AssetInitialize();
	mesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/barrel1.aab");
	mesh2 = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/barrel2.aab");
	mesh3 = AB::AssetCreateMeshAAB(asset_mgr, "../assets/barrels/barrel3.aab");
	plane = AB::AssetCreateMeshAAB(asset_mgr, "../assets/Plane.aab");
	cubeMesh = AB::AssetCreateMeshAAB(asset_mgr, "../assets/Cube.aab");
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

void Render() {
	AB::InputBeginFrame(g_Input);
	
	if (g_cam_w) {
		cam_pos = hpm::Add(hpm::Multiply(cam_front, 0.2), cam_pos);
	}
	if (g_cam_s) {
		cam_pos = hpm::Subtract(cam_pos, hpm::Multiply(cam_front, 0.2));
	}

	if (g_cam_a) {
		hpm::Vector3 right = hpm::Normalize(hpm::Cross(cam_front, { 0, 1, 0 }));
		cam_pos = hpm::Subtract(cam_pos, hpm::Multiply(right, 0.2));
	}
	if (g_cam_d) {
		hpm::Vector3 right = hpm::Normalize(hpm::Cross(cam_front, { 0, 1, 0 }));
		cam_pos = hpm::Add(hpm::Multiply(right, 0.2), cam_pos);
	}

	cam_front.x = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Cos(hpm::ToRadians(yaw));
	cam_front.y = hpm::Sin(hpm::ToRadians(pitch));
	cam_front.z = hpm::Cos(hpm::ToRadians(pitch)) * hpm::Sin(hpm::ToRadians(yaw));
	cam_front = hpm::Normalize(cam_front);
	AB::RendererSetCamera(g_Renderer, cam_front, cam_pos);

	auto tr = hpm::Translation({ 0, 0, 0 });
	auto cubeTransform = Translation({0, 0, 25});
	cubeTransform = Scale(cubeTransform, {2.5, 2.5, 27.5});
	AB::RendererSubmit(g_Renderer, plane, &tr);
	AB::RendererSubmit(g_Renderer, mesh, &tr);
	AB::RendererSubmit(g_Renderer, mesh2, &tr);
	AB::RendererSubmit(g_Renderer, mesh3, &tr);
	AB::RendererSubmit(g_Renderer, cubeMesh, &cubeTransform);

	//DEBUG_OVERLAY_PUSH_SLIDER("x", &light.direction.x, -1, 1);
	//DEBUG_OVERLAY_PUSH_SLIDER("y", &light.direction.y, -1, 1);
	//DEBUG_OVERLAY_PUSH_SLIDER("z", &light.direction.z, -1, 1);

	//DEBUG_OVERLAY_PUSH_SLIDER("amb", &light.ambient.x, 0, 1);
	//DEBUG_OVERLAY_PUSH_VAR("amb" , light.ambient.x);
	//DEBUG_OVERLAY_PUSH_SLIDER("dif", &light.diffuse.x, 0, 1);
	//DEBUG_OVERLAY_PUSH_SLIDER("spc", &light.specular.x, 0, 1);

	//DEBUG_OVERLAY_PUSH_SLIDER("light0 diff", &(plights[0].diffuse), 0, 1);
	//DEBUG_OVERLAY_PUSH_SLIDER("light0 pos", &(plights[0].position), -10, 10);

	//DEBUG_OVERLAY_PUSH_SLIDER("light1 diff", &(plights[1].diffuse), 0, 1);
	//DEBUG_OVERLAY_PUSH_SLIDER("light1 pos", &(plights[1].position), -10, 10);
	DEBUG_OVERLAY_PUSH_SLIDER("gamma", &g_Gamma, -10, 10);
	DEBUG_OVERLAY_PUSH_VAR("gamma ", g_Gamma);
	DEBUG_OVERLAY_PUSH_SLIDER("Exp", &g_Exposure, -2, 5);
	DEBUG_OVERLAY_PUSH_VAR("Exp", g_Exposure);
	DEBUG_OVERLAY_PUSH_SLIDER("I", &redLightInt, 0, 100);
	DEBUG_OVERLAY_PUSH_VAR("I", redLightInt);
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

	DEBUG_OVERLAY_PUSH_SLIDER("l", &lpos, 0, 55);

	light.ambient = { light.ambient.x, light.ambient.x ,light.ambient.x };
	light.diffuse = { light.diffuse.x , light.diffuse.x , light.diffuse.x };
	light.specular = { light.specular.x, light.specular.x, light.specular.x };
	plights[0] = {lpos, {0.1, 0.1, 0.1}, {10, 10, 10}, {}, 0.22, 0.20};
	//plights[0] = {{0, 0, 45.5}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.2}, {0.0,0.0, 0.0}, 0.22, 0.20};
	plights[1] = {{-1.4, -1.9, 9.0}, {}, {redLightInt, 0, 0}, {}, 0, 1};
	plights[2] = {{0, -1.8, 4.0}, {}, {0, 0, 0.2}, {}, 0, 1};
	plights[3] = {{0.8, -1.7, 6.0}, {}, {0.0, 0.1, 0.0}, {}, 0, 1};
	//AB::RendererSetDirectionalLight(g_Renderer, &light);
	AB::RendererSubmitPointLight(g_Renderer, &plights[0]);
	AB::RendererSubmitPointLight(g_Renderer, &plights[1]);
	AB::RendererSubmitPointLight(g_Renderer, &plights[2]);
	AB::RendererSubmitPointLight(g_Renderer, &plights[3]);

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
