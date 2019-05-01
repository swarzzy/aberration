#include "../CodeLoader.h"
#include "../Common.h"

#include <Windows.h>

#if defined(_MSC_VER)
const char* GAME_CODE_DLL_NAME = "Game.dll";
const char* TEMP_GAME_CODE_DLL_NAME = "Game_temp.dll";
#elif defined(__clang__)
const char* GAME_CODE_DLL_NAME = "Game.dll";
const char* TEMP_GAME_CODE_DLL_NAME = "Game_temp.dll";
#else
# error Unsupported compiler
#endif

namespace AB
{
	static void GameInitDummy(MemoryArena*, PlatformState*)
	{
		AB_CORE_WARN("Game code is not loaded.");
	}

	static void GameReloadDummy(MemoryArena*, PlatformState*)
	{
		AB_CORE_WARN("Game code is not loaded.");
	}
	
	
	static void GameUpdateDummy(MemoryArena*, PlatformState*)
	{
		AB_CORE_WARN("Game code is not loaded.");		
	}
	
	static void GameRenderDummy(MemoryArena*, PlatformState*)
	{
		AB_CORE_WARN("Game code is not loaded.");
	}
	
	struct GameCodeImpl
	{
		HMODULE libHandle;	
	};

	GameCode* AllocateGameCodeStruct(MemoryArena* memoryArena)
	{
		GameCode* gameCode;
		gameCode = (GameCode*)PushSize(memoryArena,
									   sizeof(GameCode),
									   alignof(GameCode));
		AB_CORE_ASSERT(gameCode, "Allocation failed");
		gameCode->impl = (GameCodeImpl*)PushSize(memoryArena,
												 sizeof(GameCodeImpl),
												 alignof(GameCodeImpl));
		AB_CORE_ASSERT(gameCode->impl, "Allocation failed");
		
		gameCode->GameInit = GameInitDummy;
		gameCode->GameReload = GameReloadDummy;
		gameCode->GameUpdate = GameUpdateDummy;
		gameCode->GameRender = GameRenderDummy;
		return gameCode;
	}

	void UnloadGameCode(GameCode* gameCode, WindowProperties* window)
	{
		char tmpLibPath[MAX_GAME_LIB_PATH];
		// TODO: Use strcat instead of FormatString.
		FormatString(tmpLibPath, MAX_GAME_LIB_PATH, "%s%s", gameCode->libDir, TEMP_GAME_CODE_DLL_NAME);
		
		//char stubPath[MAX_GAME_LIB_PATH];
		//FormatString(stubPath, MAX_GAME_LIB_PATH, "%s%s", gameCode->libDir, "foo");
		WindowRegisterInputManager(window, nullptr, {});
		FreeLibrary(gameCode->impl->libHandle);
		
		gameCode->GameInit = GameInitDummy;
		gameCode->GameReload = GameReloadDummy;
		gameCode->GameUpdate = GameUpdateDummy;
		gameCode->GameRender = GameRenderDummy;
		//MoveFile(tmpLibPath, stubPath);
		DeleteFile(tmpLibPath);
	}
	
	b32 UpdateGameCode(GameCode* gameCode, WindowProperties* window)
	{
		b32 updated = false;
		WIN32_FIND_DATA findData;
		HANDLE findHandle = FindFirstFile(gameCode->libFullPath, &findData);
		if (findHandle != INVALID_HANDLE_VALUE)
		{
			FindClose(findHandle);
			FILETIME fileTime = findData.ftLastWriteTime;
			u64 writeTime = ((u64)0 | fileTime.dwLowDateTime) | ((u64)0 | fileTime.dwHighDateTime) << 32;
			if (writeTime != gameCode->libLastChangeTime)
			{
				UnloadGameCode(gameCode, window);

				char buff[MAX_GAME_LIB_PATH];
				FormatString(buff, MAX_GAME_LIB_PATH, "%s%s",
							 gameCode->libDir, TEMP_GAME_CODE_DLL_NAME);
				auto result = CopyFile(gameCode->libFullPath, buff, FALSE);
				if (result)
				{
					gameCode->impl->libHandle = LoadLibrary(buff);//
					if (gameCode->impl->libHandle)
					{
						GameInitFn* gameInit = (GameInitFn*)GetProcAddress(gameCode->impl->libHandle,
																		   "GameInit");
						GameReloadFn* gameReload = (GameReloadFn*)GetProcAddress(gameCode->impl->libHandle,
																				 "GameReload");
						GameUpdateFn* gameUpdate = (GameUpdateFn*)GetProcAddress(gameCode->impl->libHandle,
																				 "GameUpdate");
						GameRenderFn* gameRender = (GameRenderFn*)GetProcAddress(gameCode->impl->libHandle,
																				 "GameRender");
						
						if (gameInit && gameReload && gameUpdate && gameRender)
						{
							gameCode->GameInit = gameInit;
							gameCode->GameReload = gameReload;
							gameCode->GameUpdate = gameUpdate;
							gameCode->GameRender = gameRender;
							updated = true;
							gameCode->libLastChangeTime = writeTime;
						}
						else
						{
							AB_CORE_ERROR("Failed to load game functions.");
						}
					}
					else
					{
						AB_CORE_ERROR("Failed to load game library.");
					}
				}
				else
				{
					AB_CORE_INFO("Waiting for game code loading.");
				}
			}
		}
		else
		{
			AB_CORE_ERROR("Game code not found");
		}
		return updated;
	}

	void SetupDirs(char* execPath, u32 execPathSize, char* execDir,
				   u32 execDirSize, char* gameLibPath, u32 gameLibPathSize)
	{
		u32 executablePathStrSize = 0;
		if (!AB::GetExecutablePath(execPath, execPathSize, &executablePathStrSize))
		{
			AB_CORE_FATAL("Too long executable path.");
		}

		char* executableDirPtr = execPath;
		for (char* ch = execPath; *ch; ch++)
		{
			if (*ch == '\\')
			{
				executableDirPtr = ch + 1;
			}
		}
		memcpy(execDir, execPath, (u64)(executableDirPtr - execPath));
		execDir[(u64)(executableDirPtr - execPath)] = '\0';

		FormatString(gameLibPath, gameLibPathSize, "%s%s", execDir, GAME_CODE_DLL_NAME);
	}
}
