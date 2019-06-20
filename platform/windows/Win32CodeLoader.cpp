#include "Win32CodeLoader.h"
#include "Win32Common.h"

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
	static void GameUpdateAndRenderDummy(MemoryArena*, PlatformState*,
										 GameUpdateAndRenderReason)
	{
		
	}
	
	void UnloadGameCode(Application* app)
	{
		char tmpLibPath[MAX_GAME_LIB_PATH];
		// TODO: Use strcat instead of FormatString.
		FormatString(tmpLibPath, MAX_GAME_LIB_PATH, "%s%s",
					 app->libDir, TEMP_GAME_CODE_DLL_NAME);
		
		//char stubPath[MAX_GAME_LIB_PATH];
		//FormatString(stubPath, MAX_GAME_LIB_PATH, "%s%s", app->libDir, "foo");
		FreeLibrary(app->libHandle);

		app->GameUpdateAndRender = GameUpdateAndRenderDummy;
		//MoveFile(tmpLibPath, stubPath);
		DeleteFile(tmpLibPath);
	}
	
	b32 UpdateGameCode(Application* app)
	{
		b32 updated = false;
		WIN32_FIND_DATA findData;
		HANDLE findHandle = FindFirstFile(app->libFullPath, &findData);
		if (findHandle != INVALID_HANDLE_VALUE)
		{
			FindClose(findHandle);
			FILETIME fileTime = findData.ftLastWriteTime;
			u64 writeTime = ((u64)0 | fileTime.dwLowDateTime) | ((u64)0 | fileTime.dwHighDateTime) << 32;
			if (writeTime != app->libLastChangeTime)
			{
				UnloadGameCode(app);

				char buff[MAX_GAME_LIB_PATH];
				FormatString(buff, MAX_GAME_LIB_PATH, "%s%s",
							 app->libDir, TEMP_GAME_CODE_DLL_NAME);
				auto result = CopyFile(app->libFullPath, buff, FALSE);
				if (result)
				{
					app->libHandle = LoadLibrary(buff);
					if (app->libHandle)
					{
						GameUpdateAndRenderFn* gameUpdateAndRender = (GameUpdateAndRenderFn*)GetProcAddress(app->libHandle, "GameUpdateAndRender");
						
						if (gameUpdateAndRender)
						{
							app->GameUpdateAndRender =
								gameUpdateAndRender;
							updated = true;
							app->libLastChangeTime = writeTime;
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
					//AB_CORE_INFO("Waiting for game code loading.");
				}
			}
		}
		else
		{
			//AB_CORE_ERROR("Game code not found");
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
