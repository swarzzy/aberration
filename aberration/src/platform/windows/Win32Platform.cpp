#include <windows.h>

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
	static bool32 UpdateGameCode(const char* libraryFullPath, const char* libraryDir);
	static void UnloadGameCode(const char* libraryDir);
}

static void _GameReconnectDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to reconnect game. No game code loaded");
}

static void _GameInitializeDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to initialize game. No game code loaded");
}

static void _GameUpdateDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to update game. No game code loaded");
}

static void _GameRenderDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to render game. No game code loaded");
}

static GameReconnectFn* _GameReconnect = _GameReconnectDummy;
static GameInitializeFn* _GameInitialize = _GameInitializeDummy;
static GameUpdateFn* _GameUpdate = _GameUpdateDummy;
static GameRenderFn* _GameRender = _GameRenderDummy;

static void SetupDirs(char* execPath, uint32 execPathSize, char* execDir, uint32 execDirSize, char* gameLibPath, uint32 gameLibPathSize) {
	uint32 executablePathStrSize = 0;
	if (!AB::GetExecutablePath(execPath, execPathSize, &executablePathStrSize)) {
		// TODO: Handle this. THIS SHOULD NOT BE in distribution build
		AB_CORE_FATAL("Too long executable path.");
	}

	char* executableDirPtr = execPath;
	for (char* ch = execPath; *ch; ch++) {
		if (*ch == '\\')
			executableDirPtr = ch + 1;
	}
	memcpy(execDir, execPath, (uint64)(executableDirPtr - execPath));
	execDir[(uint64)(executableDirPtr - execPath)] = '\0';

	
	AB::FormatString(gameLibPath, gameLibPathSize, "%s%s", execDir, GAME_CODE_DLL_NAME);
}

namespace AB {

	static HMODULE g_GameCodeDLL = nullptr;
	uint64 g_LastWriteTime = 0;

	static bool32 UpdateGameCode(const char* libraryFullPath, const char* libraryDir) {
		bool32 updated = false;
		WIN32_FIND_DATA findData;
		HANDLE findHandle = FindFirstFile(libraryFullPath, &findData);
		if (findHandle != INVALID_HANDLE_VALUE) {
			FindClose(findHandle);
			FILETIME fileTime = findData.ftLastWriteTime;
			uint64 writeTime = ((uint64)0 | fileTime.dwLowDateTime) | ((uint64)0 | fileTime.dwHighDateTime) << 32;
			if (writeTime != g_LastWriteTime) {
				updated = true;
				UnloadGameCode(libraryDir);

				DeleteFile(TEMP_GAME_CODE_DLL_NAME);
				char buff[280];
				FormatString(buff, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);
				auto result = CopyFile(libraryFullPath, buff, FALSE);
				if (result) {
					g_GameCodeDLL = LoadLibrary(TEMP_GAME_CODE_DLL_NAME);
					if (g_GameCodeDLL) {
						auto gameReconnect = (GameReconnectFn*)GetProcAddress(g_GameCodeDLL, "GameReconnect");
						auto gameUpdate = (GameUpdateFn*)GetProcAddress(g_GameCodeDLL, "GameUpdate");
						auto gameRender = (GameRenderFn*)GetProcAddress(g_GameCodeDLL, "GameRender");
						auto gameInitialize = (GameInitializeFn*)GetProcAddress(g_GameCodeDLL, "GameInitialize");
						if (gameUpdate && gameRender && gameInitialize && gameReconnect) {
							_GameReconnect = gameReconnect;
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
		return updated;
	}

	static void UnloadGameCode(const char* libraryDir) {
		char buff[280];
		FormatString(buff, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);

		FreeLibrary(g_GameCodeDLL);
		g_GameCodeDLL = nullptr;
		_GameReconnect = _GameReconnectDummy;
		_GameInitialize = _GameInitializeDummy;
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