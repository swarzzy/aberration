#pragma once
#include "AB.h"

namespace AB
{
	constexpr u32 MAX_GAME_LIB_PATH = 256;

	struct MemoryArena;
	struct PlatformState;
	
	typedef void(GameInitFn)(MemoryArena*, PlatformState*);
	typedef void(GameReloadFn)(MemoryArena*, PlatformState*);
	typedef void(GameUpdateFn)(MemoryArena*, PlatformState*);
	typedef void(GameRenderFn)(MemoryArena*, PlatformState*);

	struct GameCode
	{
		GameInitFn* GameInit;
		GameReloadFn* GameReload;
		GameUpdateFn* GameUpdate;
		GameRenderFn* GameRender;
		u64 libLastChangeTime;
		char libFullPath[MAX_GAME_LIB_PATH];
		char libDir[MAX_GAME_LIB_PATH];
		struct GameCodeImpl* impl;
	};

	GameCode* AllocateGameCodeStruct(MemoryArena* memoryArena);
	b32 UpdateGameCode(GameCode* gameCode, WindowProperties* window);
	void UnloadGameCode(GameCode* gameCode, WindowProperties* window);
	b32 GetExecutablePath(char* buffer,
						  u32 bufferSizeBytes, u32* bytesWritten);
	void SetupDirs(char* execPath, u32 execPathSize, char* execDir,
				   u32 execDirSize, char* gameLibPath, u32 gameLibPathSize);
}
