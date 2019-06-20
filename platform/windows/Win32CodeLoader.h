#pragma once
#include "Shared.h"

namespace AB
{
	constexpr u32 MAX_GAME_LIB_PATH = 256;

	struct MemoryArena;
	struct PlatformState;
	struct Application;
	
	typedef void(GameUpdateAndRenderFn)(MemoryArena*, PlatformState*,
										GameUpdateAndRenderReason);
	
	b32 UpdateGameCode(Application* app);
	void UnloadGameCode(Application* app);
	b32 GetExecutablePath(char* buffer,
						  u32 bufferSizeBytes, u32* bytesWritten);
	void SetupDirs(char* execPath, u32 execPathSize, char* execDir,
				   u32 execDirSize, char* gameLibPath, u32 gameLibPathSize);
}
