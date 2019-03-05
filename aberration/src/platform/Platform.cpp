#include "Platform.h"
#include "utils/DebugTools.h"
#include "Window.h"
#include "renderer/Renderer2D.h"
#include "renderer/Renderer3D.h"

#if defined(AB_PLATFORM_WINDOWS)
#include "windows/Win32Platform.cpp"
#elif defined(AB_PLATFORM_LINUX)
#include "unix/UnixPlatform.cpp"
#endif

#define GameReconnect _GameReconnect
#define GameInitialize _GameInitialize
#define GameUpdate _GameUpdate
#define GameRender _GameRender

// TODO: Make some king of interface between platform and client
namespace Client {
	// TODO: Move all these globals to some struct
	static AB::DebugOverlayProperties* g_DebugOverlay = nullptr;

	static void DebugOverlayPushString(const char* string) {
		AB::DebugOverlayPushString(g_DebugOverlay, string);
	}

	static void DebugOverlayPushVar(const char* title, hpm::Vector2 vec) {
		AB::DebugOverlayPushVar(g_DebugOverlay, title, vec);
	}

	static void DebugOverlayPushVar(const char* title, hpm::Vector3 vec) {
		AB::DebugOverlayPushVar(g_DebugOverlay, title, vec);
	}

	static void DebugOverlayPushVar(const char* title, hpm::Vector4 vec) {
		AB::DebugOverlayPushVar(g_DebugOverlay, title, vec);
	}

	static void DebugOverlayPushVar(const char* title, float32 var) {
		AB::DebugOverlayPushVar(g_DebugOverlay, title, var);
	}

	static void DebugOverlayPushVar(const char* title, int32 var) {
		AB::DebugOverlayPushVar(g_DebugOverlay, title, var);
	}

	static void DebugOverlayPushVar(const char* title, uint32 var) {
		AB::DebugOverlayPushVar(g_DebugOverlay, title, var);
	}

	void DebugOverlayPushSlider(const char* title, float32* val, float32 min, float32 max) {
		AB::DebugOverlayPushSlider(g_DebugOverlay, title, val, min, max);
	}

	void DebugOverlayPushSlider(const char* title, int32* val, int32 min, int32 max) {
		AB::DebugOverlayPushSlider(g_DebugOverlay, title, val, min, max);
	}

	void DebugOverlayPushSlider(const char* title, uint32* val, uint32 min, uint32 max) {
		AB::DebugOverlayPushSlider(g_DebugOverlay, title, val, min, max);
	}
}

static void _LoadEngineFunctions(AB::Engine* context) {
	context->fillRectangleTexture = AB::Renderer2D::FillRectangleTexture;
	context->fillRectangleColor = AB::Renderer2D::FillRectangleColor;
	context->loadTexture = AB::Renderer2D::LoadTexture;
	context->freeTexture = AB::Renderer2D::FreeTexture;
	context->textureCreateRegion = AB::Renderer2D::TextureCreateRegion;
	context->windowSetKeyCallback = AB::Window::SetKeyCallback;
	context->debugDrawString = AB::Renderer2D::DebugDrawString;
	context->debugDrawStringW = AB::Renderer2D::DebugDrawString;
	context->getStringBoundingRect = AB::Renderer2D::GetStringBoundingRect;
	context->glGetFunctions = AB::GL::GetFunctions;
	context->log = AB::Log;
	context->logAssert = AB::LogAssert;
	context->formatString = AB::FormatString;
	context->printString = AB::PrintString;
	context->loadBMP = AB::LoadBMP;
	context->deleteBitmap = AB::DeleteBitmap;
	context->SetMouseMoveCallback = AB::Window::SetMouseMoveCallback;
	context->windowGetMousePositionCallback = AB::Window::GetMousePosition;
	context->GetKeyState = AB::Window::GetKeyState;
	context->GetMouseButtonState = AB::Window::GetMouseButtonState;
	context->GetWindowSize = AB::Window::GetSize;
	context->SetMousePosition = AB::Window::SetMousePosition;
	context->SetFocusCallback = AB::Window::SetFocusCallback;
	context->WindowActive = AB::Window::WindowActive;
	context->ShowCursor = AB::Window::ShowCursor;
	// Debug overlay
	context->DebugOverlayPushStr = Client::DebugOverlayPushString;
	context->DebugOverlayPushV2 = Client::DebugOverlayPushVar;
	context->DebugOverlayPushV3 = Client::DebugOverlayPushVar;
	context->DebugOverlayPushV4 = Client::DebugOverlayPushVar;
	context->DebugOverlayPushF32 = Client::DebugOverlayPushVar;
	context->DebugOverlayPushI32 = Client::DebugOverlayPushVar;
	context->DebugOverlayPushU32 = Client::DebugOverlayPushVar;
	context->DebugOverlayPushSliderF32 = Client::DebugOverlayPushSlider;
	context->DebugOverlayPushSliderI32 = Client::DebugOverlayPushSlider;
	context->DebugOverlayPushSliderU32 = Client::DebugOverlayPushSlider;

}

static AB::ApplicationProperties* g_AppProperties = nullptr;

static constexpr int64 UPDATE_INTERVAL = 16000;
static constexpr int64 SECOND_INTERVAL = 1000000;

float32 vertices[288] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};

int main()
{
	AB_CORE_INFO("Aberration engine");
	const uint32 executablePathBufferSize = 256;
	const uint32 executableDirBufferSize = 256;
	const uint32 gameLibraryPathSize = 280;
	char executablePath[executablePathBufferSize];
	char executableDir[executableDirBufferSize];
	char gameLibraryPath[gameLibraryPathSize];

	SetupDirs(executablePath, executablePathBufferSize, executableDir, executableDirBufferSize, gameLibraryPath, gameLibraryPathSize);

	AB::Window::Create("Aberration", 1280, 720);
	AB::Window::EnableVSync(true);

	AB::Renderer2D::Initialize(1280, 720);

	AB::Engine* engine = (AB::Engine*)std::malloc(sizeof(AB::Engine));
	memset(engine, 0, sizeof(AB::Engine));

	_LoadEngineFunctions(engine);

	AB::GameContext* gameContext = (AB::GameContext*)std::malloc(sizeof(AB::GameContext));
	memset(gameContext, 0, sizeof(AB::GameContext));

	AB::UpdateGameCode(gameLibraryPath, executableDir);
	GameReconnect(engine, gameContext);
	GameInitialize(engine, gameContext);

	// TODO: Custom allocator
	g_AppProperties = (AB::ApplicationProperties*)malloc(sizeof(AB::ApplicationProperties));
	memset(g_AppProperties, 0, sizeof(AB::ApplicationProperties));

	g_AppProperties->runningTime = AB::GetCurrentRawTime();

	Client::g_DebugOverlay = AB::CreateDebugOverlay();
	AB::DebugOverlayEnableMainPane(Client::g_DebugOverlay, true);

	int64 updateTimer = UPDATE_INTERVAL;
	int64 tickTimer = SECOND_INTERVAL;
	uint32 updatesSinceLastTick = 0;

	AB::Renderer3DInit();
	int32 material = AB::Renderer3DCreateMaterial("../../../assets/test.bmp", "../../../assets/spec.bmp", 32);
	int32 material1 = AB::Renderer3DCreateMaterial("../../../assets/spec.bmp", "../../../assets/spec.bmp", 32);

	int32 mesh = AB::Renderer3DCreateMesh(vertices, 288);
	float32 pitch = 0;
	float32 yaw = 0;

	AB::DirectionalLight light = {};

	while (AB::Window::IsOpen()) {
		if (tickTimer <= 0) {
			tickTimer = SECOND_INTERVAL;
			g_AppProperties->ups = updatesSinceLastTick;
			updatesSinceLastTick = 0;
			AB::UpdateDebugOverlay(Client::g_DebugOverlay);
		}
		if (updateTimer <= 0) {
			updateTimer = UPDATE_INTERVAL;
			updatesSinceLastTick++;
			if (AB::UpdateGameCode(gameLibraryPath, executableDir)) {
				GameReconnect(engine, gameContext);
			}
			GameUpdate(engine, gameContext);
		}
		AB::DrawDebugOverlay(Client::g_DebugOverlay);
		// TODO: Temporary here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		AB::Renderer3DSubmit(mesh, material, &hpm::Translation({1, 0, 1}));
		AB::Renderer3DSubmit(mesh, material1, &hpm::Translation({ -1, 0, 2 }));

		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "pitch", &pitch, -180.0f, 180.0f);
		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "yaw", &yaw, -360.0f, 360.0f);

		AB::DebugOverlayPushVar(Client::g_DebugOverlay, "pitch", pitch);
		AB::DebugOverlayPushVar(Client::g_DebugOverlay, "yaw", yaw);

		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "x", &light.direction.x, -1, 1);
		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "y", &light.direction.y, -1, 1);
		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "z", &light.direction.z, -1, 1);

		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "amb", &light.ambient.x, 0, 1);
		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "dif", &light.diffuse.x, 0, 1);
		AB::DebugOverlayPushSlider(Client::g_DebugOverlay, "spc", &light.specular.x, 0, 1);

		light.ambient = { light.ambient.x, light.ambient.x ,light.ambient.x };
		light.diffuse = { light.diffuse.x , light.diffuse.x , light.diffuse.x };
		light.specular = { light.specular.x, light.specular.x, light.specular.x };

		AB::Renderer3DSetDirectionalLight(&light);

		AB::Renderer3DSetCamera(pitch, yaw, { 0, 0, -3 });
		AB::Renderer3DRender();
		//GameRender(engine, gameContext);

		AB::Renderer2D::Flush();
		AB::Window::PollEvents();
		AB::Window::SwapBuffers();

		int64 currentTime = AB::GetCurrentRawTime();
		g_AppProperties->frameTime = currentTime - g_AppProperties->runningTime;
		g_AppProperties->runningTime = currentTime;
		tickTimer -= g_AppProperties->frameTime;
		updateTimer -= g_AppProperties->frameTime;
		g_AppProperties->fps = SECOND_INTERVAL / g_AppProperties->frameTime;
	}
	AB::UnloadGameCode(executableDir);
	return 0;
}

namespace AB {

	const ApplicationProperties* GetAppProperties() {
		return g_AppProperties;
	}

	uint32 DateTime::ToString(char* buffer, uint32 bufferSize) {
		if (hour < 24 && minute < 60 && seconds < 60) {
			if (bufferSize >= DATETIME_STRING_SIZE) {
				int32 written = FormatString(buffer, bufferSize, "%02u16:%02u16:%02u16", hour, minute, seconds);
				return DATETIME_STRING_SIZE - 1;
			}
		}
		else {
			if (bufferSize >= DATETIME_STRING_SIZE) {
				FormatString(buffer, bufferSize, "00:00:00");
				return DATETIME_STRING_SIZE - 1;
			}
		}
		return 1;
	}
}