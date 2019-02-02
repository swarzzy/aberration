#include "../Platform.h"
#include "src/utils/Log.h"
#include "platform/Window.h"
#include "renderer/Renderer2D.h"

#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>


const char* GAME_CODE_DLL_NAME = "libSandbox.so";
const char* TEMP_GAME_CODE_DLL_NAME = "libSandbox_temp.so";

static void _GameInitializeDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	// TODO: fix Log.h include loop
	//AB_CORE_ERROR("Failed to update game. No game code loaded");
}

static void _GameUpdateDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	// TODO: fix Log.h include loop
	//AB_CORE_ERROR("Failed to update game. No game code loaded");
}

static void _GameRenderDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	//AB_CORE_ERROR("Failed to render game. No game code loaded");
}

static GameInitializeFn* _GameInitialize = _GameInitializeDummy;
static GameUpdateFn* _GameUpdate = _GameUpdateDummy;
static GameRenderFn* _GameRender = _GameRenderDummy;

#define GameInitialize _GameInitialize
#define GameUpdate _GameUpdate
#define GameRender _GameRender

int main()
{
	const uint32 executablePathBufferSize = 256;
	const uint32 executableDirBufferSize = 256;
	char executablePath[executablePathBufferSize];
	char executableDir[executableDirBufferSize];

	uint32 executablePathStrSize = 0;
	if (!AB::GetExecutablePath(executablePath, executablePathBufferSize, &executablePathStrSize)) {
		// TODO: Handle this. THIS SHOULD NOT BE in distribution build
		AB_CORE_FATAL("Too long executable path.");
	}
	printf("%s", executablePath);

	char* executableDirPtr = executablePath;
	for (char* ch = executablePath; *ch; ch++) {
		if (*ch == '/')
			executableDirPtr = ch + 1;
	}
	memcpy(executableDir, executablePath, (uint64)(executableDirPtr - executablePath));
	executableDir[(uint64)(executableDirPtr - executablePath)] = '\0';

	const uint32 gameLibraryPathSize = 280;
	char gameLibraryPath[gameLibraryPathSize];
	snprintf(gameLibraryPath, gameLibraryPathSize,"%s%s", executableDir, GAME_CODE_DLL_NAME);


	AB::Window::Create("Aberration", 800, 600);
	AB::Window::EnableVSync(true);

	AB::Renderer2D::Initialize(800, 600);
	AB::Renderer2D::LoadTexture("A:\\dev\\ab_nonrepo\\test.bmp");

	AB::Engine* engine = (AB::Engine*)std::malloc(sizeof(AB::Engine));
	memset(engine, 0, sizeof(AB::Engine));
	engine->drawRectangle = AB::Renderer2D::DrawRectangle;
	engine->windowSetKeyCallback = AB::Window::SetKeyCallback;

	AB::GameContext* gameContext = (AB::GameContext*)std::malloc(sizeof(AB::GameContext));
	memset(gameContext, 0, sizeof(AB::GameContext));

	while (AB::Window::IsOpen()) {
		AB::UpdateGameCode(gameLibraryPath, executableDir);
		GameUpdate(engine, gameContext);
		GameRender(engine, gameContext);
		AB::Renderer2D::Flush();
		AB::Window::PollEvents();
		AB::Window::SwapBuffers();

	}
	AB::UnloadGameCode(executableDir);
	return 0;
}

namespace AB {

	static void* g_GameCodeDLL = nullptr;
	time_t g_LastWriteTime = 0;

	void UpdateGameCode(const char* libraryFullPath, const char* libraryDir) {
        char tempLibName[280];
		struct stat fileAttribs;
		if (stat(libraryFullPath, &fileAttribs) == 0) {
			time_t writeTime = fileAttribs.st_mtime;
			if (writeTime != g_LastWriteTime) {

				UnloadGameCode(libraryDir);

				remove(TEMP_GAME_CODE_DLL_NAME);

				snprintf(tempLibName, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);
				uint32 libRead;
				void* libData = DebugReadFile(libraryFullPath, &libRead);
				if (libData) {
					if (DebugWriteFile(tempLibName, libData, libRead)) {
					    DebugFreeFileMemory(libData);
						g_GameCodeDLL = dlopen(tempLibName, RTLD_LAZY | RTLD_LOCAL);
						if (g_GameCodeDLL) {
							auto gameUpdate = (GameUpdateFn *)dlsym(g_GameCodeDLL, "GameUpdate");
							auto gameRender = (GameRenderFn *)dlsym(g_GameCodeDLL, "GameRender");
							auto gameInitialize = (GameInitializeFn *)dlsym(g_GameCodeDLL, "GameInitialize");
							if (gameUpdate && gameRender && gameInitialize) {
								_GameInitialize = gameInitialize;
								_GameUpdate = gameUpdate;
								_GameRender = gameRender;

								g_LastWriteTime = writeTime;
							} else {
								AB_CORE_ERROR("Failed to load game functions.");
							}
						} else {
							AB_CORE_ERROR("Failed to load game library.");
						}
					} else {
                        AB_CORE_ERROR("Failed to load game library.");
					}
				} else {
                    AB_CORE_ERROR("Failed to load game library.");
				}
			}
		} else {
			AB_CORE_ERROR("Game code not found");
		}
	}

	void UnloadGameCode(const char* libraryDir) {
		char buff[280];
		snprintf(buff, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);

		if (g_GameCodeDLL)
		    dlclose(g_GameCodeDLL);
		g_GameCodeDLL = nullptr;
		_GameUpdate = _GameUpdateDummy;
		_GameRender = _GameRenderDummy;
		remove(buff);
	}

	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten) {
		ssize_t len = readlink("/proc/self/exe", buffer, bufferSizeBytes - 1);
		if (len == bufferSizeBytes -1) {
			*bytesWritten = len + 1;
			return false;
		}
		if (len != -1) {
			buffer[len] = '\0';
			*bytesWritten = len + 1;
			return true;
		} else {
			*bytesWritten = 0;
			return  false;
		}
	}

	String DateTime::ToString() {
		if (hour < 24 && minute < 60 && seconds < 60) {
			char buff[DATETIME_STRING_SIZE];
			sprintf(buff, "%02d:%02d:%02d", hour, minute, seconds);
			return String(buff);
		}
		return ("00:00:00");
	}

	AB_API void GetLocalTime(DateTime& datetime) {
		std::time_t t = std::time(nullptr);
		auto tm = std::localtime(&t);

		datetime.year = static_cast<uint16>(tm->tm_year);
		datetime.month = static_cast<uint16>(tm->tm_mon);
		datetime.dayOfWeek = static_cast<uint16>(tm->tm_wday);
		datetime.day = static_cast<uint16>(tm->tm_mday);
		datetime.hour = static_cast<uint16>(tm->tm_hour);
		datetime.minute = static_cast<uint16>(tm->tm_min);
		datetime.seconds = static_cast<uint16>(tm->tm_sec);
		datetime.milliseconds = 0;
	}

	AB_API void* DebugReadFile(const char* filename, uint32* bytesRead) {
		void* ptr = nullptr;
		*bytesRead = 0;
		int fileHandle = open(filename, O_RDONLY);
		if (fileHandle) {
			off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
			if (fileEnd) {
				lseek(fileHandle, 0, SEEK_SET);
				void* data = std::malloc(fileEnd);
				if (data) {
					ssize_t result = read(fileHandle, data, fileEnd);
					if (result == fileEnd) {
						ptr = data;
						*bytesRead = (uint32)result;
					}
					else {
						std::free(data);
						AB_CORE_WARN("File reading error. File: ", filename, ". Failed read data from file");
					}
				}
				else {
					AB_CORE_WARN("File reading error. File: ", filename, ". Memory allocation failed");
				}
			}
			else {
				AB_CORE_WARN("File reading error. File: ", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: ", filename, ". Failed to open file");
			return ptr;
		}
		close(fileHandle);
		return ptr;
	}

	AB_API void DebugFreeFileMemory(void* memory) {
		if (memory) {
			std::free(memory);
		}
	}

	AB_API bool32 DebugWriteFile(const char* filename, void* data, uint32 dataSize) {
		bool32 result = false;
		int fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IRWXU | S_IRGRP);
		if (fileHandle) {
			ssize_t written = write(fileHandle, data, dataSize);
			if (written == dataSize) {
				result = true;
			}
			else {
				AB_CORE_WARN("File reading error. File: ", filename, ". Write operation failed");
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: ", filename, ". Failed to open file");
			return false;
		}
		close(fileHandle);
		return result;
	}
}