#include "Platform.h"
#include "utils/DebugTools.h"
#include "Window.h"
#include "renderer/Renderer2D.h"

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
}

static AB::ApplicationProperties* g_AppProperties = nullptr;

static constexpr int64 UPDATE_INTERVAL = 16000;
static constexpr int64 SECOND_INTERVAL = 1000000;

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

	AB::Window::Create("Aberration", 800, 600);
	AB::Window::EnableVSync(true);

	AB::Renderer2D::Initialize(800, 600);

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
		GameRender(engine, gameContext);

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