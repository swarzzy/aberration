#pragma once
#include "AB.h"
#include "Shared.h"
#include "PlatformOpenGL.h"

namespace AB{

	struct GameCode;
	struct WindowProperties;

	struct Application
	{
		MemoryArena* systemMemory;
		MemoryArena* gameMemory;
		void* gameStaticStorage;
		PlatformState state;
		WindowProperties* window;
		GameCode* gameCode;
	};

	AB_API Application* AppCreate(MemoryArena* sysMemory);
	AB_API void AppRun(Application* app);
}
