
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>

namespace AB {
	static bool32 UpdateGameCode(const char* libraryFullPath, const char* libraryDir);
	static void UnloadGameCode(const char* libraryDir);
}

const char* GAME_CODE_DLL_NAME = "libSandbox.so";
const char* TEMP_GAME_CODE_DLL_NAME = "libSandbox_temp.so";

static void _GameReconnectDummy(AB::Engine* engine, AB::GameContext* gameContext) {
	AB_CORE_ERROR("Failed to reconnect game. No game code loaded");
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
		if (*ch == '/')
			executableDirPtr = ch + 1;
	}
	memcpy(execDir, execPath, (uint64)(executableDirPtr - execPath));
	execDir[(uint64)(executableDirPtr - execPath)] = '\0';

	AB::FormatString(gameLibPath, gameLibPathSize, "%s%s", execDir, GAME_CODE_DLL_NAME);
}

namespace AB {

	static void* g_GameCodeDLL = nullptr;
	time_t g_LastWriteTime = 0;

	static bool32 UpdateGameCode(const char* libraryFullPath, const char* libraryDir) {
        bool32 updated = false;
        char tempLibName[280];
		struct stat fileAttribs;
		if (stat(libraryFullPath, &fileAttribs) == 0) {
			time_t writeTime = fileAttribs.st_mtime;
			if (writeTime != g_LastWriteTime) {
				updated = true;
				UnloadGameCode(libraryDir);

				remove(TEMP_GAME_CODE_DLL_NAME);

				AB::FormatString(tempLibName, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);
				uint32 libRead;
				void* libData = DebugReadFile(libraryFullPath, &libRead);
				if (libData) {
					if (DebugWriteFile(tempLibName, libData, libRead)) {
					    DebugFreeFileMemory(libData);
						g_GameCodeDLL = dlopen(tempLibName, RTLD_LAZY | RTLD_LOCAL);
						if (g_GameCodeDLL) {
							auto gameReconnect = (GameReconnectFn *)dlsym(g_GameCodeDLL, "GameReconnect");
							auto gameUpdate = (GameUpdateFn *)dlsym(g_GameCodeDLL, "GameUpdate");
							auto gameRender = (GameRenderFn *)dlsym(g_GameCodeDLL, "GameRender");
							auto gameInitialize = (GameInitializeFn *)dlsym(g_GameCodeDLL, "GameInitialize");
							if (gameUpdate && gameRender && gameInitialize && gameReconnect) {
								_GameInitialize = gameInitialize;
								_GameUpdate = gameUpdate;
								_GameRender = gameRender;
								_GameReconnect = gameReconnect;

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
		return updated;
	}

	static void UnloadGameCode(const char* libraryDir) {
		char buff[280];
		AB::FormatString(buff, 280, "%s%s", libraryDir, TEMP_GAME_CODE_DLL_NAME);

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

	int64 GetCurrentRawTime() {
		int64 time = 0;
		struct timespec tp;
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &tp) == 0) {
			time = ((int64)tp.tv_sec * 1000000) + ((int64)tp.tv_nsec / 1000);
		}
		return time;
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
						AB_CORE_WARN("File reading error. File: %s. Failed read data from file", filename);
					}
				}
				else {
					AB_CORE_WARN("File reading error. File: %s. Memory allocation failed", filename);
				}
			}
			else {
				AB_CORE_WARN("File reading error. File: %s", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: %s. Failed to open file.", filename);
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
				AB_CORE_WARN("File reading error. File: %s. Write operation failed.", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: %s. Failed to open file", filename);
			return false;
		}
		close(fileHandle);
		return result;
	}
}