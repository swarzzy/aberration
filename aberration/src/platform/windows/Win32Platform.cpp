#include "../Platform.h"
#include <windows.h>
#include <strsafe.h>
#include "src/utils/Log.h"
#include "src/platform/Window.h"
#include "src/renderer/Renderer2D.h"
#include "platform/API/OpenGL/ABOpenGL.h"
#include "utils/DebugTools.h"

// TODO: They are shouldn`t be hardcoded
#if defined(_MSC_VER)
const char* GAME_CODE_DLL_NAME = "Sandbox.dll";
const char* TEMP_GAME_CODE_DLL_NAME = "Sandbox_temp.dll";
#elif defined(__clang__)
const char* GAME_CODE_DLL_NAME = "Sandbox.dll";
const char* TEMP_GAME_CODE_DLL_NAME = "Sandbox_temp.dll";
#else
# error Unsupported compiler
#endif

namespace AB {
	static void UpdateGameCode(const char* libraryFullPath, const char* libraryDir);
	static void UnloadGameCode(const char* libraryDir);
}

static void _GameInitializeDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to update game. No game code loaded");
}

static void _GameUpdateDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to update game. No game code loaded");
}

static void _GameRenderDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to render game. No game code loaded");
}

static GameInitializeFn* _GameInitialize = _GameInitializeDummy;
static GameUpdateFn* _GameUpdate = _GameUpdateDummy;
static GameRenderFn* _GameRender = _GameRenderDummy;

#define GameInitialize _GameInitialize
#define GameUpdate _GameUpdate
#define GameRender _GameRender

static void _LoadEngineFunctions(AB::Engine* context) {
	context->fillRectangleTexture	= AB::Renderer2D::FillRectangleTexture;
	context->fillRectangleColor		= AB::Renderer2D::FillRectangleColor;
	context->loadTexture			= AB::Renderer2D::LoadTexture;
	context->freeTexture			= AB::Renderer2D::FreeTexture;
	context->textureCreateRegion	= AB::Renderer2D::TextureCreateRegion;
	context->windowSetKeyCallback	= AB::Window::SetKeyCallback;
	context->debugDrawString		= AB::Renderer2D::DebugDrawString;
	context->getStringBoundingRect	= AB::Renderer2D::GetStringBoundingRect;
}

static AB::ApplicationProperties* g_AppProperties = nullptr;

static constexpr int64 UPDATE_INTERVAL = 16000;
static constexpr int64 SECOND_INTERVAL = 1000000;

int main()
{
	AB_CORE_INFO("Aberration engine");
	const uint32 executablePathBufferSize = 256;
	const uint32 executableDirBufferSize = 256;
	char executablePath[executablePathBufferSize];
	char executableDir[executableDirBufferSize];

	uint32 executablePathStrSize = 0;
	if (!AB::GetExecutablePath(executablePath, executablePathBufferSize, &executablePathStrSize)) {
		// TODO: Handle this. THIS SHOULD NOT BE in distribution build
		AB_CORE_FATAL("Too long executable path.");
	}

	char* executableDirPtr = executablePath;
	for (char* ch = executablePath; *ch; ch++) {
		if (*ch == '\\')
			executableDirPtr = ch + 1;
	}
	memcpy(executableDir, executablePath, (uint64)(executableDirPtr - executablePath));
	executableDir[(uint64)(executableDirPtr - executablePath)] = '\0';

	const uint32 gameLibraryPathSize = 280;
	char gameLibraryPath[gameLibraryPathSize];
	AB::FormatString(gameLibraryPath, gameLibraryPathSize,"%s%s", executableDir, GAME_CODE_DLL_NAME);


	AB::Window::Create("Aberration", 800, 600);
	AB::Window::EnableVSync(true);

	AB::Renderer2D::Initialize(800, 600);

	AB::Engine* engine = (AB::Engine*)std::malloc(sizeof(AB::Engine));
	memset(engine, 0, sizeof(AB::Engine));

	_LoadEngineFunctions(engine);

	AB::GameContext* gameContext = (AB::GameContext*)std::malloc(sizeof(AB::GameContext));
	memset(gameContext, 0, sizeof(AB::GameContext));

	AB::UpdateGameCode(gameLibraryPath, executableDir);
	GameInitialize(engine, gameContext);

	// TODO: Custom allocator
	g_AppProperties = (AB::ApplicationProperties*)malloc(sizeof(AB::ApplicationProperties));
	memset(g_AppProperties, 0, sizeof(AB::ApplicationProperties));

	g_AppProperties->runningTime = AB::GetCurrentRawTime();

	AB::DebugOverlayProperties debugOverlay = {};

	int64 updateTimer = UPDATE_INTERVAL;
	int64 tickTimer = SECOND_INTERVAL;
	uint32 updatesSinceLastTick = 0;

	while (AB::Window::IsOpen()) {

		if (tickTimer <= 0) {
			tickTimer = SECOND_INTERVAL;
			g_AppProperties->ups = updatesSinceLastTick;
			updatesSinceLastTick = 0;
			AB::UpdateDebugOverlay(&debugOverlay);
		}

		if(updateTimer <= 0) {
			updateTimer = UPDATE_INTERVAL;
			updatesSinceLastTick++;
			AB::UpdateGameCode(gameLibraryPath, executableDir);
			GameUpdate(engine, gameContext);
		}

		AB::DrawDebugOverlay(&debugOverlay);
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

	static HMODULE g_GameCodeDLL = nullptr;
	uint64 g_LastWriteTime = 0;

	static void UpdateGameCode(const char* libraryFullPath, const char* libraryDir) {
		WIN32_FIND_DATA findData;
		HANDLE findHandle = FindFirstFile(libraryFullPath, &findData);
		if (findHandle != INVALID_HANDLE_VALUE) {
			FindClose(findHandle);
			FILETIME fileTime = findData.ftLastWriteTime;
			uint64 writeTime = ((uint64)0 | fileTime.dwLowDateTime) | ((uint64)0 | fileTime.dwHighDateTime) << 32;
			if (writeTime != g_LastWriteTime) {

				UnloadGameCode(libraryDir);

				DeleteFile(TEMP_GAME_CODE_DLL_NAME);
				char buff[280];
				FormatString(buff, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);
				auto result = CopyFile(libraryFullPath, buff, FALSE);
				if (result) {
					g_GameCodeDLL = LoadLibrary(TEMP_GAME_CODE_DLL_NAME);
					if (g_GameCodeDLL) {
						auto gameUpdate = (GameUpdateFn*)GetProcAddress(g_GameCodeDLL, "GameUpdate");
						auto gameRender = (GameRenderFn*)GetProcAddress(g_GameCodeDLL, "GameRender");
						auto gameInitialize = (GameInitializeFn*)GetProcAddress(g_GameCodeDLL, "GameInitialize");
						if (gameUpdate && gameRender && gameInitialize) {
							_GameInitialize = gameInitialize;
							_GameUpdate = gameUpdate;
							_GameRender = gameRender;

							g_LastWriteTime = writeTime;
						}
						else {
							AB_CORE_ERROR("Failed to load game functions.");
						}
					}
					else {
						AB_CORE_ERROR("Failed to load game library.");
					}
				} else {
					AB_CORE_INFO("Waiting for game code loading.");
				}
			}
		} else {
			AB_CORE_ERROR("Game code not found");
		}
	}

	static void UnloadGameCode(const char* libraryDir) {
		char buff[280];
		FormatString(buff, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);

		FreeLibrary(g_GameCodeDLL);
		g_GameCodeDLL = nullptr;
		_GameUpdate = _GameUpdateDummy;
		_GameRender = _GameRenderDummy;
		DeleteFile(buff);
	}

	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten) {
		auto written = GetModuleFileName(NULL, buffer, bufferSizeBytes);
		auto result = GetLastError();
		if (result == ERROR_INSUFFICIENT_BUFFER)
			return false;
		else
			return true;
	}

	int64 GetCurrentRawTime() {
		static LARGE_INTEGER frequency = {};
		if (!frequency.QuadPart) {
			QueryPerformanceFrequency(&frequency);
		}

		int64 time = 0;
		LARGE_INTEGER currentTime = {};
		if (QueryPerformanceCounter(&currentTime)) {
			AB_CORE_ASSERT(currentTime.QuadPart && frequency.QuadPart, "Failed to get values from windows performance counters.");
			time = (currentTime.QuadPart * 1000000) / frequency.QuadPart;
		}
		return  time;
	}

	const ApplicationProperties* GetAppProperties() {
		return g_AppProperties;
	}

	uint32 DateTime::ToString(char* buffer, uint32 bufferSize) {
		if (hour < 24 && minute < 60 && seconds < 60) {
			if (bufferSize >= DATETIME_STRING_SIZE) {
				int32 written = FormatString(buffer, bufferSize, "%02u16:%02u16:%02u16", hour, minute, seconds);
				return DATETIME_STRING_SIZE - 1;
			}
		} else {
			if (bufferSize >= DATETIME_STRING_SIZE) {
				FormatString(buffer, bufferSize, "00:00:00");
				return DATETIME_STRING_SIZE - 1;
			}
		}
		return 1;
	}

	void GetLocalTime(DateTime& datetime) {
		SYSTEMTIME time;
		GetLocalTime(&time);

		datetime.year = time.wYear;
		datetime.month = time.wMonth;
		datetime.dayOfWeek = time.wDayOfWeek;
		datetime.day = time.wDay;
		datetime.hour =	time.wHour;
		datetime.minute = time.wMinute;
		datetime.seconds = time.wSecond;
		datetime.milliseconds = time.wMilliseconds;
	}


	// TODO: set bytesRead to zero if reading failed
	void* DebugReadFile(const char* filename, uint32* bytesRead) {
		void* bitmap = nullptr;
		LARGE_INTEGER fileSize = {0};
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE) {
			if (GetFileSizeEx(fileHandle, &fileSize)) {
				if (fileSize.QuadPart > 0xffffffff) {
					AB_CORE_ERROR("Can`t read >4GB file.");
					CloseHandle(fileHandle);
					return nullptr;
				}
				bitmap = std::malloc(fileSize.QuadPart);
				if (bitmap) {
					DWORD read;
					if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
						AB_CORE_ERROR("Failed to read file.");
						DebugFreeFileMemory(bitmap);
						bitmap = nullptr;
					}
				}
			}
			CloseHandle(fileHandle);
		}
		*bytesRead = (uint32)fileSize.QuadPart;
		return  bitmap;
	}

	void DebugFreeFileMemory(void* memory) {
		if (memory) {
			std::free(memory);
		}
	}

	bool32 DebugWriteFile(const char* filename, void* data, uint32 dataSize) {
		HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE) {
			DWORD bytesWritten;
			if (WriteFile(fileHandle, data, dataSize, &bytesWritten, 0) && (dataSize == bytesWritten)) {
				CloseHandle(fileHandle);
				return true;
			}
		}
		CloseHandle(fileHandle);
		return false;
	}
}
